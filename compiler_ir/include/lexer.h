#pragma once
#include "token.h"
#include "symtable.h"
#include "automata.h"
#include <vector>
#include <string>

class Lexer {
public:
    Lexer(const std::string& sourceCode);
    
    // Lexer -> Parser 接口
    std::vector<Token> tokenize();
    FrontSymbolTable& getSymbolTable() { return symTable; }
    
    // 严格按规范格式打印
    static void printTokens(const std::vector<Token>& tokens);
    void printSymbolTable();

private:
    std::string source;
    int currentPos;
    int currentLine;
    int currentCol;
    FrontSymbolTable symTable;
    
    std::vector<DFAState> minDfa;
    int dfaStart;

    void buildAndMinimizeAutomata();
    void buildNFA(AutomataCore& ac, int startState);
    void addStringNFA(AutomataCore& ac, int startState, const std::string& str, TokenType type);
    
    bool isKeywordIgnoreCase(const std::string& id);
    std::string toLower(const std::string& str);
    void skipWhitespaceAndComments();
};