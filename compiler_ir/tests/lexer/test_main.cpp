#include "lexer/lexer.h"
#include <iostream>

int main() {
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

    return 0;
}