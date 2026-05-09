#include "lexer/lexer.h"
#include <iostream>

int main() {
    // 包含各类情况的综合单测输入样例
    std::string sourceCode = R"(
        int Main() {
            /* Test case for lexical analysis */
            CONST float PI = 3.14159;
            int a = 10;
            if (a >= 5 && PI < 4.0) {
                return a * 2;
            } else {
                return 0;
            }
        }
    )";

    std::cout << "--- 开始词法分析 ---" << std::endl;
    Lexer lexer(sourceCode);
    std::vector<Token> tokens = lexer.tokenize();
    
    // 按格式打印规约结果
    Lexer::printTokens(tokens);

    std::cout << "\n--- 符号表抽取结果 ---" << std::endl;
    // 验证符号表维护逻辑（你的另一项交付职责）
    SymbolTable& st = lexer.getSymbolTable();
    SymbolEntry entry;
    if (st.lookup("Main", entry)) {
        std::cout << "Found IDN: " << entry.name << ", Kind: " << entry.kind << std::endl;
    }
    if (st.lookup("PI", entry)) {
        std::cout << "Found IDN: " << entry.name << ", Kind: " << entry.kind << std::endl;
    }

    return 0;
}