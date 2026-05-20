#pragma once
#include <string>
#include <unordered_map>
#include <vector>

// 涓ユ牸閬靛惊鍓嶇绗﹀彿琛ㄨ鑼冨瓧娈?
struct FrontSymbolEntry {
    std::string name;
    std::string kind;  // "var", "func", "array" 绛?
    std::string type;  // "int", "float", "void" 绛?
    int scope;         // 浣滅敤鍩熷眰绾?(Lexer榛樿缁?锛岀敱Parser缁存姢)
    std::string value; // 鍙€夌殑甯搁噺鍊?
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

