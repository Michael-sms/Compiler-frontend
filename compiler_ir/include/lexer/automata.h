#pragma once
#include <vector>
#include <map>
#include <set>
#include "common/token.h"

// NFA 状态定义
struct NFAState {
    std::map<char, std::vector<int>> transitions;
    std::vector<int> epsilonTransitions;
    bool isAccept = false;
    TokenType acceptType = TokenType::ERROR;
};

// DFA 状态定义
struct DFAState {
    std::map<char, int> transitions;
    bool isAccept = false;
    TokenType acceptType = TokenType::ERROR;
};

class AutomataCore {
public:
    // NFA构建接口
    int addNFAState();
    void addTransition(int from, int to, char c);
    void addEpsilonTransition(int from, int to);
    void setAccept(int state, TokenType type);
    
    // NFA 到 DFA 确定化 (子集构造法)
    std::vector<DFAState> nfaToDfa(int nfaStartState);
    
    // DFA 最小化 (Moore划分算法)
    std::vector<DFAState> minimizeDfa(const std::vector<DFAState>& dfa, int& startState);

private:
    std::vector<NFAState> nfaStates;
    std::set<int> epsilonClosure(const std::set<int>& states);
    std::set<int> move(const std::set<int>& states, char c);
};