#pragma once
#include <string>

// 涓ユ牸閬靛惊瑙勮寖涓殑鏋氫妇锛屼娇鐢?enum class 閬垮厤 EOF 瀹忓啿绐?
enum class TokenType {
    KW,     // 鍏抽敭瀛?
    OP,     // 杩愮畻绗?
    SE,     // 界符
    IDN,    // 鏍囪瘑绗?
    INT,    // 整数
    FLOAT,  // 娴偣鏁?
    EOF_TOK,// 缁撴潫绗?(閬垮厤涓嶤瀹廍OF缁濆鍐茬獊锛岃緭鍑烘椂杞负EOF)
    ERROR   // 错误单词
};

// 严格遵循：type、lexeme、line、column
struct Token {
    TokenType type;
    std::string lexeme;
    int line;
    int column;
};

