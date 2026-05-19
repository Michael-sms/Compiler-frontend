#include "ast.h"
#include "lexer.h"
#include "slr_parser_a.h"
#include "slr_parser_b.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

namespace {
void runLexerDemo() {
    // 包含各类情况的综合单测输入样例
    std::string sourceCode = R"(
        INT Main() {
            /* Lexer test */
            const float PI = 3.14;
            int a = 10;
            if (a >= 5 && PI <= 4.0) {
                return a * 2;
            } ELSE {
                return 0;
            }
        }
    )";

    std::cout << "--- 词法分析规约序列输出 ---" << std::endl;
    Lexer lexer(sourceCode);
    std::vector<Token> tokens = lexer.tokenize();

    // 按格式打印规约结果
    Lexer::printTokens(tokens);

    // 打印符号表（供检查）
    lexer.printSymbolTable();
}

void runParserATables(const std::string& outputDir) {
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
}

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
    Lexer lexer(sourceCode);
    std::vector<Token> tokens = lexer.tokenize();

    parser_b::SLRParserB parser = parser_b::SLRParserB::BuildDefault();
    parser_b::ParseResult result = parser.parse(tokens, true, std::cout);

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

void printUsage(const char* exe) {
    std::cout << "Usage:\n";
    std::cout << "  " << exe << " lexer\n";
    std::cout << "  " << exe << " parserA [output_dir]\n";
    std::cout << "  " << exe << " parserB [input_file]\n";
}
}

int main(int argc, char** argv) {
    if (argc < 2) {
        printUsage(argv[0]);
        return 0;
    }

    std::string mode = argv[1];
    if (mode == "lexer") {
        runLexerDemo();
        return 0;
    }

    if (mode == "parserA") {
        std::string outputDir = "./parser_a_output";
        if (argc >= 3) {
            outputDir = argv[2];
        }
        runParserATables(outputDir);
        return 0;
    }

    if (mode == "parserB") {
        std::string sourceCode;
        if (argc >= 3) {
            sourceCode = readFileOrEmpty(argv[2]);
            if (sourceCode.empty()) {
                std::cerr << "Failed to read input file: " << argv[2] << "\n";
                return 1;
            }
        } else {
            sourceCode = "int a;";
        }
        runParserBDemo(sourceCode);
        return 0;
    }

    printUsage(argv[0]);
    return 1;
}