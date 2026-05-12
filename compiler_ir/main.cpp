#include "lexer.h"
#include "slr_parser_a.h"
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

    std::string sourceCode;

    // 约定：若传入参数，则按文件输入；否则使用内置样例
    if (argc >= 2) {
        sourceCode = readFileOrEmpty(argv[1]);
        if (sourceCode.empty()) {
            std::cerr << "Failed to read input file: " << argv[1] << std::endl;
            return 1;
        }
    } else {
        // 默认样例：覆盖关键字、运算符、界符、整型/浮点等常见情况
        sourceCode = R"(
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
    }

    // 当前阶段：仅进行词法分析与符号表输出
    // 后续阶段：将 tokens 传入 Parser，得到 AST，再交由 IR 生成模块
    std::cout << "--- Lexer Output ---" << std::endl;
    Lexer lexer(sourceCode);
    std::vector<Token> tokens = lexer.tokenize();
    Lexer::printTokens(tokens);
    lexer.printSymbolTable();

    return 0;
}