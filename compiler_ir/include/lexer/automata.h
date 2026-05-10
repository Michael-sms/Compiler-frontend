#pragma once
#include <vector>
#include <map>
#include <set>
#include "common/token.h"

struct NFAState {
    std::map<char, std::vector<int>> transitions;
    std::vector<int> epsilonTransitions;
    bool isAccept = false;
    TokenType acceptType = TokenType::ERROR;
};

struct DFAState {
    std::map<char, int> transitions;
    bool isAccept = false;
    TokenType acceptType = TokenType::ERROR;
};

class AutomataCore {
public:
    int addNFAState();
    void addTransition(int from, int to, char c);
    void addEpsilonTransition(int from, int to);
    void setAccept(int state, TokenType type);
    
    // NFA->DFA 确定化
    std::vector<DFAState> nfaToDfa(int nfaStartState);
    // DFA 最小化
    std::vector<DFAState> minimizeDfa(const std::vector<DFAState>& dfa, int& startState);

private:
    std::vector<NFAState> nfaStates;
    std::set<int> epsilonClosure(const std::set<int>& states);
    std::set<int> move(const std::set<int>& states, char c);
};