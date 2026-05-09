#pragma once
#include <string>
#include <unordered_map>

struct SymbolEntry {
    std::string name;
    std::string kind;  // "variable", "function", "keyword" 等
    std::string type;  // "int", "float" 等
    int scope;         // 作用域
};

class SymbolTable {
public:
    void insert(const std::string& name, const SymbolEntry& entry) {
        if (table.find(name) == table.end()) {
            table[name] = entry;
        }
    }
    bool lookup(const std::string& name, SymbolEntry& entry) {
        auto it = table.find(name);
        if (it != table.end()) {
            entry = it->second;
            return true;
        }
        return false;
    }
private:
    std::unordered_map<std::string, SymbolEntry> table;
};