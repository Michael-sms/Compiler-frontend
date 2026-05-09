#pragma once
#include "common/token.h"
#include "common/symtable.h"
#include "lexer/automata.h"
#include <vector>
#include <string>

class Lexer {
public:
    Lexer(const std::string& sourceCode);
    std::vector<Token> tokenize();
    SymbolTable& getSymbolTable() { return symTable; }
    static void printTokens(const std::vector<Token>& tokens);

private:
    std::string source;
    int currentPos;
    int currentLine;
    int currentCol;
    SymbolTable symTable;
    
    std::vector<DFAState> minDfa;
    int dfaStart;

    void buildAndMinimizeAutomata();
    void buildNFA(AutomataCore& ac, int startState);
    void addStringNFA(AutomataCore& ac, int startState, const std::string& str, TokenType type);
    
    bool isKeyword(const std::string& id);
    void skipWhitespaceAndComments();
};