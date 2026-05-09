#include "lexer/lexer.h"
#include <iostream>
#include <cctype>
#include <algorithm>

Lexer::Lexer(const std::string& sourceCode) 
    : source(sourceCode), currentPos(0), currentLine(1), currentCol(1) {
    buildAndMinimizeAutomata();
}

// 帮助函数：向NFA中添加精确字符串匹配（用于OP和SE）
void Lexer::addStringNFA(AutomataCore& ac, int startState, const std::string& str, TokenType type) {
    int curr = startState;
    for (size_t i = 0; i < str.length(); ++i) {
        int next = ac.addNFAState();
        ac.addTransition(curr, next, str[i]);
        curr = next;
    }
    ac.setAccept(curr, type);
}

void Lexer::buildNFA(AutomataCore& ac, int startState) {
    // 1. IDN: (a-z|A-Z|_) (a-z|A-Z|_|0-9)*
    int idnStart = ac.addNFAState();
    int idnBody = ac.addNFAState();
    ac.addEpsilonTransition(startState, idnStart);
    auto addLetters = [&](int from, int to) {
        for(char c='a'; c<='z'; ++c) ac.addTransition(from, to, c);
        for(char c='A'; c<='Z'; ++c) ac.addTransition(from, to, c);
        ac.addTransition(from, to, '_');
    };
    auto addDigits = [&](int from, int to) {
        for(char c='0'; c<='9'; ++c) ac.addTransition(from, to, c);
    };
    addLetters(idnStart, idnBody);
    addLetters(idnBody, idnBody);
    addDigits(idnBody, idnBody);
    ac.setAccept(idnBody, TokenType::IDN);

    // 2. INT: (0-9)+
    int intStart = ac.addNFAState();
    int intBody = ac.addNFAState();
    ac.addEpsilonTransition(startState, intStart);
    addDigits(intStart, intBody);
    addDigits(intBody, intBody);
    ac.setAccept(intBody, TokenType::INT);

    // 3. FLOAT: (0-9)+ . (0-9)+
    int floatDot = ac.addNFAState();
    int floatBody = ac.addNFAState();
    ac.addTransition(intBody, floatDot, '.'); // 复用int的路径
    addDigits(floatDot, floatBody);
    addDigits(floatBody, floatBody);
    ac.setAccept(floatBody, TokenType::FLOAT);

    // 4. OP & SE (通过字符串路径直接构建NFA)
    std::vector<std::string> ops = {"+", "-", "*", "/", "%", "=", ">", "<", "==", "<=", ">=", "!=", "&&", "||"};
    for (const auto& op : ops) addStringNFA(ac, startState, op, TokenType::OP);

    std::vector<std::string> ses = {"(", ")", "{", "}", ";", ","};
    for (const auto& se : ses) addStringNFA(ac, startState, se, TokenType::SE);
}

void Lexer::buildAndMinimizeAutomata() {
    AutomataCore ac;
    int nfaStart = ac.addNFAState();
    buildNFA(ac, nfaStart);
    
    // NFA -> DFA
    std::vector<DFAState> dfa = ac.nfaToDfa(nfaStart);
    // DFA -> MinDFA
    minDfa = ac.minimizeDfa(dfa, dfaStart);
}

void Lexer::skipWhitespaceAndComments() {
    while (currentPos < source.length()) {
        char c = source[currentPos];
        if (std::isspace(c)) {
            if (c == '\n') { currentLine++; currentCol = 1; }
            else { currentCol++; }
            currentPos++;
        } else if (c == '/' && currentPos + 1 < source.length() && source[currentPos+1] == '*') {
            // 跳过多行注释 /* ... */
            currentPos += 2; currentCol += 2;
            while (currentPos + 1 < source.length() && !(source[currentPos] == '*' && source[currentPos+1] == '/')) {
                if (source[currentPos] == '\n') { currentLine++; currentCol = 1; }
                else { currentCol++; }
                currentPos++;
            }
            currentPos += 2; currentCol += 2;
        } else {
            break;
        }
    }
}

bool Lexer::isKeyword(const std::string& id) {
    std::string lowerId = id;
    std::transform(lowerId.begin(), lowerId.end(), lowerId.begin(), ::tolower);
    const std::vector<std::string> kws = {"int", "void", "return", "const", "main", "float", "if", "else"};
    return std::find(kws.begin(), kws.end(), lowerId) != kws.end();
}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;
    
    while (currentPos < source.length()) {
        skipWhitespaceAndComments();
        if (currentPos >= source.length()) break;

        int state = dfaStart;
        int lastAcceptState = -1;
        int lastAcceptPos = -1;
        
        int tempPos = currentPos;
        // 最大贪婪匹配 (Maximal Munch)
        while (tempPos < source.length()) {
            char c = source[tempPos];
            if (minDfa[state].transitions.count(c) == 0) break;
            
            state = minDfa[state].transitions[c];
            if (minDfa[state].isAccept) {
                lastAcceptState = state;
                lastAcceptPos = tempPos;
            }
            tempPos++;
        }

        if (lastAcceptPos != -1) {
            std::string lexeme = source.substr(currentPos, lastAcceptPos - currentPos + 1);
            TokenType type = minDfa[lastAcceptState].acceptType;

            // 关键字查表判断
            if (type == TokenType::IDN && isKeyword(lexeme)) {
                type = TokenType::KW;
            }

            tokens.push_back({type, lexeme, currentLine, currentCol});
            
            // 写入符号表 (标识符)
            if (type == TokenType::IDN) {
                symTable.insert(lexeme, {lexeme, "variable", "unknown", 0});
            }

            currentCol += lexeme.length();
            currentPos = lastAcceptPos + 1;
        } else {
            // 处理无法识别的错误字符
            tokens.push_back({TokenType::ERROR, std::string(1, source[currentPos]), currentLine, currentCol});
            currentPos++; currentCol++;
        }
    }
    
    tokens.push_back({TokenType::END_FILE, "EOF", currentLine, currentCol});
    return tokens;
}

void Lexer::printTokens(const std::vector<Token>& tokens) {
    for (const auto& t : tokens) {
        if (t.type == TokenType::END_FILE) break;
        
        std::string typeStr;
        switch(t.type) {
            case TokenType::KW: typeStr = "KW"; break;
            case TokenType::OP: typeStr = "OP"; break;
            case TokenType::SE: typeStr = "SE"; break;
            case TokenType::IDN: typeStr = "IDN"; break;
            case TokenType::INT: typeStr = "INT"; break;
            case TokenType::FLOAT: typeStr = "FLOAT"; break;
            case TokenType::ERROR: typeStr = "ERROR"; break;
            default: typeStr = "UNKNOWN"; break;
        }
        
        std::string outLexeme = t.lexeme;
        // 根据规范，关键字不区分大小写，这里输出统一转小写
        if (t.type == TokenType::KW) {
            std::transform(outLexeme.begin(), outLexeme.end(), outLexeme.begin(), ::tolower);
        }

        std::cout << t.lexeme << "\t<" << typeStr << "," << outLexeme << ">\n";
    }
}