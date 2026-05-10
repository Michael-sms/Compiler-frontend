#pragma once
#include <string>

// 严格遵循规范中的枚举，使用 enum class 避免 EOF 宏冲突
enum class TokenType {
    KW,     // 关键字
    OP,     // 运算符
    SE,     // 界符
    IDN,    // 标识符
    INT,    // 整数
    FLOAT,  // 浮点数
    EOF_TOK,// 结束符 (避免与C宏EOF绝对冲突，输出时转为EOF)
    ERROR   // 错误单词
};

// 严格遵循：type、lexeme、line、column
struct Token {
    TokenType type;
    std::string lexeme;
    int line;
    int column;
};