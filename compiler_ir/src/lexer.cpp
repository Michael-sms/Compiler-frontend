#include "lexer.h"
#include <iostream>
#include <cctype>
#include <algorithm>

Lexer::Lexer(const std::string& sourceCode) 
    : source(sourceCode), currentPos(0), currentLine(1), currentCol(1) {
    buildAndMinimizeAutomata();
}

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
    // 1. IDN
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

    // 2. INT
    int intStart = ac.addNFAState();
    int intBody = ac.addNFAState();
    ac.addEpsilonTransition(startState, intStart);
    addDigits(intStart, intBody);
    addDigits(intBody, intBody);
    ac.setAccept(intBody, TokenType::INT);

    // 3. FLOAT
    int floatDot = ac.addNFAState();
    int floatBody = ac.addNFAState();
    ac.addTransition(intBody, floatDot, '.'); 
    addDigits(floatDot, floatBody);
    addDigits(floatBody, floatBody);
    ac.setAccept(floatBody, TokenType::FLOAT);

    // 4. OP: 规范要求 + - * / % = > < == <= >= != && ||
    std::vector<std::string> ops = {"+", "-", "*", "/", "%", "=", ">", "<", "==", "<=", ">=", "!=", "&&", "||"};
    for (const auto& op : ops) addStringNFA(ac, startState, op, TokenType::OP);

    // 5. SE: 规范要求 ( ) { } ; ,
    std::vector<std::string> ses = {"(", ")", "{", "}", ";", ","};
    for (const auto& se : ses) addStringNFA(ac, startState, se, TokenType::SE);
}

void Lexer::buildAndMinimizeAutomata() {
    AutomataCore ac;
    int nfaStart = ac.addNFAState();
    buildNFA(ac, nfaStart);
    std::vector<DFAState> dfa = ac.nfaToDfa(nfaStart);
    minDfa = ac.minimizeDfa(dfa, dfaStart);
}

std::string Lexer::toLower(const std::string& str) {
    std::string lowerStr = str;
    std::transform(lowerStr.begin(), lowerStr.end(), lowerStr.begin(), ::tolower);
    return lowerStr;
}

bool Lexer::isKeywordIgnoreCase(const std::string& id) {
    const std::vector<std::string> kws = {"int", "void", "return", "const", "main", "float", "if", "else"};
    return std::find(kws.begin(), kws.end(), toLower(id)) != kws.end();
}

void Lexer::skipWhitespaceAndComments() {
    while (currentPos < source.length()) {
        char c = source[currentPos];
        if (std::isspace(c)) {
            if (c == '\n') { currentLine++; currentCol = 1; }
            else { currentCol++; }
            currentPos++;
        } else if (c == '/' && currentPos + 1 < source.length() && source[currentPos+1] == '*') {
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

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;
    
    while (currentPos < source.length()) {
        skipWhitespaceAndComments();
        if (currentPos >= source.length()) break;

        int state = dfaStart;
        int lastAcceptState = -1;
        int lastAcceptPos = -1;
        
        int tempPos = currentPos;
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

            // 处理不区分大小写的关键字
            if (type == TokenType::IDN && isKeywordIgnoreCase(lexeme)) {
                type = TokenType::KW;
            }

            tokens.push_back({type, lexeme, currentLine, currentCol});
            
            // 维护前端符号表 (仅加入标识符，Parser后续补充类型和作用域)
            if (type == TokenType::IDN) {
                symTable.insert(lexeme, {lexeme, "var", "unknown", 0, ""});
            }

            currentCol += lexeme.length();
            currentPos = lastAcceptPos + 1;
        } else {
            tokens.push_back({TokenType::ERROR, std::string(1, source[currentPos]), currentLine, currentCol});
            currentPos++; currentCol++;
        }
    }
    
    // 接口统一规范：末尾追加 EOF
    tokens.push_back({TokenType::EOF_TOK, "EOF", currentLine, currentCol});
    return tokens;
}

// 规范：[单词符号] [TAB] <[种别],[内容]>，关键字不区分大小写
void Lexer::printTokens(const std::vector<Token>& tokens) {
    for (const auto& t : tokens) {
        if (t.type == TokenType::EOF_TOK) break;
        
        std::string typeStr;
        switch(t.type) {
            case TokenType::KW: typeStr = "KW"; break;
            case TokenType::OP: typeStr = "OP"; break;
            case TokenType::SE: typeStr = "SE"; break;
            case TokenType::IDN: typeStr = "IDN"; break;
            case TokenType::INT: typeStr = "INT"; break;
            case TokenType::FLOAT: typeStr = "FLOAT"; break;
            case TokenType::ERROR: typeStr = "ERROR"; break;
            default: break;
        }
        
        std::string content = t.lexeme;
        // 关键字输出统一为小写（实现不区分大小写的要求）
        if (t.type == TokenType::KW) {
            std::transform(content.begin(), content.end(), content.begin(), ::tolower);
        }

        std::cout << t.lexeme << "\t<" << typeStr << "," << content << ">\n";
    }
}

void Lexer::printSymbolTable() {
    std::cout << "\n=== Front Symbol Table (IDN) ===\n";
    std::cout << "Name\tKind\tType\tScope\n";
    for (const auto& key : symTable.getKeys()) {
        FrontSymbolEntry entry;
        symTable.lookup(key, entry);
        std::cout << entry.name << "\t" << entry.kind << "\t" << entry.type << "\t" << entry.scope << "\n";
    }
}