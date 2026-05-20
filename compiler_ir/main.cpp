#include "lexer.h"
#include "SLRParser.h"
#include "IRGenerator.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#ifdef _WIN32
#include <windows.h>
#endif

// 读取输入文件（失败则返回空字符串）
namespace {
std::string readFileOrEmpty(const std::string& path) {
    std::ifstream in(path, std::ios::in);
    if (!in.is_open()) {
        return {};
    }
    std::ostringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

void runParserBDemo(const std::string& sourceCode) {
    std::cout << "--- Parser B Trace ---\n";
    std::cout.flush();
    Lexer lexer(sourceCode);
    std::vector<Token> tokens = lexer.tokenize();

    parser_b::ParseResult result;
    try {
        parser_b::SLRParserB parser = parser_b::SLRParserB::BuildDefault();
        result = parser.parse(tokens, true, std::cout);
    } catch (const std::exception& ex) {
        std::cout << "\n--- Exception ---\n" << ex.what() << "\n";
        return;
    } catch (...) {
        std::cout << "\n--- Exception ---\n<unknown>\n";
        return;
    }

    std::cout << "\n--- Reduction Sequence ---\n";
    parser_b::SLRParserB::PrintReductionSequence(result.reductions, std::cout);

    std::cout << "\n--- Errors ---\n";
    parser_b::SLRParserB::PrintErrors(result.errors, std::cout);

    std::cout << "\n--- AST (Preorder) ---\n";
    if (result.root) {
        frontend::DumpAstPreorder(result.root, std::cout);
    } else {
        std::cout << "<empty>\n";
    }
}

void runIRDemo(const std::string& sourceCode, const std::string& outputPath) {
    Lexer lexer(sourceCode);
    std::vector<Token> tokens = lexer.tokenize();

    parser_b::ParseResult result;
    try {
        parser_b::SLRParserB parser = parser_b::SLRParserB::BuildDefault();
        result = parser.parse(tokens, false, std::cout);
    } catch (const std::exception& ex) {
        std::cout << "\n--- Exception ---\n" << ex.what() << "\n";
        return;
    } catch (...) {
        std::cout << "\n--- Exception ---\n<unknown>\n";
        return;
    }

    if (!result.accepted || !result.errors.empty()) {
        std::cout << "\n--- Errors ---\n";
        parser_b::SLRParserB::PrintErrors(result.errors, std::cout);
        return;
    }

    IRGenerator generator("sysy2025_compiler");
    generator.generate(result.root);

    if (!generator.errors().empty()) {
        std::cout << "\n--- IR Errors ---\n";
        for (const auto& err : generator.errors()) {
            std::cout << "- " << err << "\n";
        }
        return;
    }

    std::ofstream out(outputPath, std::ios::out | std::ios::trunc);
    if (!out.is_open()) {
        std::cout << "Failed to write IR file: " << outputPath << "\n";
        return;
    }
    out << generator.print();
    std::cout << "IR written to: " << outputPath << "\n";
}

std::string defaultSample() {
    return R"(
    int main() {
    int a = 10;
    int b = 20;
    int c = a + b;
    if (c > 0) {
        return c;
    } else {
        return 0;
    }
}
        )";
}
}

int main(int argc, char** argv) {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    if (argc >= 2 && std::string(argv[1]) == "parserA") {
        std::string outputDir = "./parser_a_output";
        if (argc >= 3) {
            outputDir = argv[2];
        }

        parser_a::SLRParserABuilder builder(parser_a::BuildDefaultCMinusGrammar(), "Program");
        builder.build();
        builder.dumpAll(outputDir);

        std::cout << "Parser A build finished.\n";
        std::cout << "Output dir: " << outputDir << "\n";
        std::cout << "Productions: " << builder.productions().size() - 1 << " (excluding augmented)\n";
        std::cout << "LR(0) states: " << builder.canonicalCollection().size() << "\n";
        std::cout << "Action entries: " << builder.actionTable().size() << "\n";
        std::cout << "Goto entries: " << builder.gotoTable().size() << "\n";
        std::cout << "Conflicts: " << builder.conflicts().size() << "\n";
        return 0;
    }

    if (argc >= 2 && std::string(argv[1]) == "parserB") {
        std::string sourceCode;
        if (argc >= 3) {
            sourceCode = readFileOrEmpty(argv[2]);
            if (sourceCode.empty()) {
                std::cerr << "Failed to read input file: " << argv[2] << std::endl;
                return 1;
            }
        } else {
            sourceCode = "int a;";
        }

        runParserBDemo(sourceCode);
        return 0;
    }

    if (argc >= 2 && std::string(argv[1]) == "ir") {
        std::string sourceCode;
        if (argc >= 3) {
            sourceCode = readFileOrEmpty(argv[2]);
            if (sourceCode.empty()) {
                std::cerr << "Failed to read input file: " << argv[2] << std::endl;
                return 1;
            }
        } else {
            sourceCode = defaultSample();
        }
        std::string outputPath = "./output.ll";
        if (argc >= 4) {
            outputPath = argv[3];
        }
        runIRDemo(sourceCode, outputPath);
        return 0;
    }

    std::string sourceCode;
    // 约定：若传入参数，则按文件输入；否则使用内置样例
    if (argc >= 2) {
        sourceCode = readFileOrEmpty(argv[1]);
        if (sourceCode.empty()) {
            std::cerr << "Failed to read input file: " << argv[1] << std::endl;
            return 1;
        }
    } else {
        // 默认样例：覆盖关键字、运算符、界符、整数/浮点等常见情况
        sourceCode = defaultSample();
    }
    // 当前阶段：词法输出与 Parser A/Parser B 表构建入口已打通
    // 后续阶段：将 AST 交由 IR 生成模块
    std::cout << "--- Lexer Output ---" << std::endl;
    Lexer lexer(sourceCode);
    std::vector<Token> tokens = lexer.tokenize();
    Lexer::printTokens(tokens);
    lexer.printSymbolTable();

    return 0;
}


