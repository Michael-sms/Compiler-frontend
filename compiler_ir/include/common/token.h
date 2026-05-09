#pragma once
#include <string>

// 遵循任务要求的枚举
enum class TokenType {
    KW,     // 关键字
    OP,     // 运算符
    SE,     // 界符
    IDN,    // 标识符
    INT,    // 整数
    FLOAT,  // 浮点数
    ERROR,  // 词法错误
    END_FILE// 结束符 (避免与宏EOF冲突)
};

struct Token {
    TokenType type;
    std::string lexeme;
    int line;
    int column;
};