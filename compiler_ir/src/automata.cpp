#include "automata.h"
#include <queue>
#include <algorithm>

int AutomataCore::addNFAState() {
    nfaStates.push_back(NFAState());
    return nfaStates.size() - 1;
}

void AutomataCore::addTransition(int from, int to, char c) {
    nfaStates[from].transitions[c].push_back(to);
}

void AutomataCore::addEpsilonTransition(int from, int to) {
    nfaStates[from].epsilonTransitions.push_back(to);
}

void AutomataCore::setAccept(int state, TokenType type) {
    nfaStates[state].isAccept = true;
    nfaStates[state].acceptType = type;
}

std::set<int> AutomataCore::epsilonClosure(const std::set<int>& states) {
    std::set<int> closure = states;
    std::queue<int> q;
    for (int s : states) q.push(s);

    while (!q.empty()) {
        int s = q.front();
        q.pop();
        for (int next : nfaStates[s].epsilonTransitions) {
            if (closure.find(next) == closure.end()) {
                closure.insert(next);
                q.push(next);
            }
        }
    }
    return closure;
}

std::set<int> AutomataCore::move(const std::set<int>& states, char c) {
    std::set<int> result;
    for (int s : states) {
        if (nfaStates[s].transitions.count(c)) {
            for (int next : nfaStates[s].transitions.at(c)) {
                result.insert(next);
            }
        }
    }
    return result;
}

std::vector<DFAState> AutomataCore::nfaToDfa(int nfaStartState) {
    std::vector<DFAState> dfa;
    std::map<std::set<int>, int> dfaStateMap;
    std::queue<std::set<int>> worklist;

    std::set<int> startSet = epsilonClosure({nfaStartState});
    dfaStateMap[startSet] = 0;
    worklist.push(startSet);
    dfa.push_back(DFAState());

    while (!worklist.empty()) {
        std::set<int> currentSet = worklist.front();
        int currentDfaState = dfaStateMap[currentSet];
        worklist.pop();

        for (int nfaState : currentSet) {
            if (nfaStates[nfaState].isAccept) {
                dfa[currentDfaState].isAccept = true;
                if (dfa[currentDfaState].acceptType == TokenType::ERROR || 
                    nfaStates[nfaState].acceptType < dfa[currentDfaState].acceptType) {
                    dfa[currentDfaState].acceptType = nfaStates[nfaState].acceptType;
                }
            }
        }

        std::set<char> allChars;
        for (int s : currentSet) {
            for (auto const& pair : nfaStates[s].transitions) {
                allChars.insert(pair.first);
            }
        }

        for (char c : allChars) {
            std::set<int> nextSet = epsilonClosure(move(currentSet, c));
            if (nextSet.empty()) continue;

            if (dfaStateMap.find(nextSet) == dfaStateMap.end()) {
                int newDfaId = dfa.size();
                dfaStateMap[nextSet] = newDfaId;
                dfa.push_back(DFAState());
                worklist.push(nextSet);
            }
            dfa[currentDfaState].transitions[c] = dfaStateMap[nextSet];
        }
    }
    return dfa;
}

std::vector<DFAState> AutomataCore::minimizeDfa(const std::vector<DFAState>& dfa, int& startState) {
    if (dfa.empty()) return {};

    std::vector<int> partition(dfa.size());
    for (size_t i = 0; i < dfa.size(); ++i) {
        if (!dfa[i].isAccept) partition[i] = 0;
        else partition[i] = static_cast<int>(dfa[i].acceptType) + 1;
    }

    bool changed = true;
    while (changed) {
        changed = false;
        std::map<std::pair<int, std::map<char, int>>, std::vector<int>> groups;
        
        for (size_t i = 0; i < dfa.size(); ++i) {
            std::map<char, int> signature;
            for (auto const& trans : dfa[i].transitions) {
                signature[trans.first] = partition[trans.second];
            }
            groups[{partition[i], signature}].push_back(i);
        }

        int newGroupId = 0;
        std::vector<int> newPartition = partition;
        for (auto const& group : groups) {
            for (int stateIndex : group.second) {
                if (newPartition[stateIndex] != newGroupId) {
                    newPartition[stateIndex] = newGroupId;
                    changed = true;
                }
            }
            newGroupId++;
        }
        partition = newPartition;
    }

    int numMinStates = *std::max_element(partition.begin(), partition.end()) + 1;
    std::vector<DFAState> minDfa(numMinStates);
    startState = partition[0]; 

    for (size_t i = 0; i < dfa.size(); ++i) {
        int minId = partition[i];
        minDfa[minId].isAccept = dfa[i].isAccept;
        minDfa[minId].acceptType = dfa[i].acceptType;
        for (auto const& trans : dfa[i].transitions) {
            minDfa[minId].transitions[trans.first] = partition[trans.second];
        }
    }
    return minDfa;
}