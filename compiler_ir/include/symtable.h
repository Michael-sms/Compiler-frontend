#pragma once
#include <string>
#include <unordered_map>
#include <vector>

// 严格遵循前端符号表规范字段
struct FrontSymbolEntry {
    std::string name;
    std::string kind;  // "var", "func", "array" 等
    std::string type;  // "int", "float", "void" 等
    int scope;         // 作用域层级 (Lexer默认给0，由Parser维护)
    std::string value; // 可选的常量值
};

class FrontSymbolTable {
public:
    // 插入符号（如果不存在则插入）
    void insert(const std::string& name, const FrontSymbolEntry& entry) {
        if (table.find(name) == table.end()) {
            table[name] = entry;
            ordered_keys.push_back(name);
        }
    }

    bool lookup(const std::string& name, FrontSymbolEntry& entry) {
        auto it = table.find(name);
        if (it != table.end()) {
            entry = it->second;
            return true;
        }
        return false;
    }

    const std::vector<std::string>& getKeys() const { return ordered_keys; }

private:
    std::unordered_map<std::string, FrontSymbolEntry> table;
    std::vector<std::string> ordered_keys; // 用于顺序打印展示
};