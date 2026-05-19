#pragma once

#include <map>
#include <ostream>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "ast.h"
#include "token.h"

namespace parser_a {

inline constexpr const char* kEpsilon = "epsilon";
inline constexpr const char* kEndMarker = "$";

struct Production {
    int id = -1;
    std::string lhs;
    std::vector<std::string> rhs;
};

struct LR0Item {
    int prod_id = -1;
    int dot_pos = 0;

    bool operator<(const LR0Item& other) const {
        if (prod_id != other.prod_id) {
            return prod_id < other.prod_id;
        }
        return dot_pos < other.dot_pos;
    }

    bool operator==(const LR0Item& other) const {
        return prod_id == other.prod_id && dot_pos == other.dot_pos;
    }
};

enum class ActionType {
    Shift,
    Reduce,
    Accept,
    Error
};

struct ActionEntry {
    ActionType type = ActionType::Error;
    int target = -1;
};

struct ConflictRecord {
    int state = -1;
    std::string symbol;
    ActionEntry existing;
    ActionEntry incoming;
    ActionEntry chosen;
    std::string resolution;
};

class SLRParserABuilder {
public:
    SLRParserABuilder(std::vector<Production> productions, std::string start_symbol);

    void build();

    const std::vector<Production>& productions() const { return all_productions_; }
    const std::map<std::string, std::set<std::string>>& first() const { return first_; }
    const std::map<std::string, std::set<std::string>>& follow() const { return follow_; }
    const std::vector<std::set<LR0Item>>& canonicalCollection() const { return canonical_collection_; }
    const std::map<std::pair<int, std::string>, ActionEntry>& actionTable() const { return action_table_; }
    const std::map<std::pair<int, std::string>, int>& gotoTable() const { return goto_table_; }
    const std::vector<ConflictRecord>& conflicts() const { return conflicts_; }

    void dumpProductions(const std::string& path) const;
    void dumpFirstFollow(const std::string& path) const;
    void dumpLR0Items(const std::string& path) const;
    void dumpActionCsv(const std::string& path) const;
    void dumpGotoCsv(const std::string& path) const;
    void dumpAll(const std::string& output_dir) const;

private:
    std::vector<Production> input_productions_;
    std::vector<Production> all_productions_;
    std::string start_symbol_;
    std::string augmented_start_symbol_ = "__START__";

    std::set<std::string> terminals_;
    std::set<std::string> nonterminals_;
    std::map<std::string, std::set<std::string>> first_;
    std::map<std::string, std::set<std::string>> follow_;

    std::vector<std::set<LR0Item>> canonical_collection_;
    std::map<std::pair<int, std::string>, int> state_transitions_;
    std::map<std::pair<int, std::string>, ActionEntry> action_table_;
    std::map<std::pair<int, std::string>, int> goto_table_;
    std::vector<ConflictRecord> conflicts_;

    std::map<int, std::size_t> prod_index_by_id_;
    std::map<std::string, std::vector<int>> productions_by_lhs_;

    void resetBuiltState();
    void buildProductionIndex();
    void computeSymbols();
    void computeFirst();
    void computeFollow();
    void buildCanonicalCollection();
    void buildTables();

    std::set<std::string> firstOfSequence(const std::vector<std::string>& sequence) const;
    std::set<LR0Item> closure(std::set<LR0Item> items) const;
    std::set<LR0Item> gotoState(const std::set<LR0Item>& items, const std::string& symbol) const;

    void setAction(int state, const std::string& symbol, const ActionEntry& incoming);

    const Production& productionById(int id) const;
    bool isTerminal(const std::string& symbol) const;
    bool isNonterminal(const std::string& symbol) const;

    std::vector<std::string> orderedTerminals() const;
    std::vector<std::string> orderedNonterminalsForGoto() const;
};

std::vector<Production> BuildDefaultCMinusGrammar();
std::string TokenToGrammarTerminal(const Token& token);
std::string ActionEntryToString(const ActionEntry& action);

}  // namespace parser_a

namespace parser_b {

struct ParseResult {
    bool accepted = false;
    std::vector<int> reductions;
    std::vector<std::string> errors;
    frontend::ASTNodePtr root;
};

class SLRParserB {
public:
    SLRParserB(std::vector<parser_a::Production> productions, std::string startSymbol);

    static SLRParserB BuildDefault();

    ParseResult parse(const std::vector<Token>& tokens, bool trace, std::ostream& out) const;

    static void PrintReductionSequence(const std::vector<int>& reductions, std::ostream& out);
    static void PrintErrors(const std::vector<std::string>& errors, std::ostream& out);

private:
    parser_a::SLRParserABuilder builder_;
    std::map<int, parser_a::Production> productionsById_;

    const parser_a::Production* findProduction(int id) const;
};

}  // namespace parser_b
