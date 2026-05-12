#include "parser/slr_parser_a.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <stdexcept>

namespace parser_a {
namespace {

bool ContainsEpsilon(const std::set<std::string>& symbols) {
    return symbols.find(kEpsilon) != symbols.end();
}

std::string Join(const std::vector<std::string>& parts, const std::string& sep) {
    std::ostringstream oss;
    for (std::size_t i = 0; i < parts.size(); ++i) {
        if (i != 0) {
            oss << sep;
        }
        oss << parts[i];
    }
    return oss.str();
}

std::string ToLowerAscii(std::string text) {
    for (char& ch : text) {
        ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
    }
    return text;
}

std::string SetToString(const std::set<std::string>& symbols) {
    std::vector<std::string> ordered(symbols.begin(), symbols.end());
    return "{ " + Join(ordered, ", ") + " }";
}

void EnsureDirectory(const std::string& output_dir) {
    if (output_dir.empty()) {
        return;
    }
#ifdef _WIN32
    const std::string command = "if not exist \"" + output_dir + "\" mkdir \"" + output_dir + "\"";
#else
    const std::string command = "mkdir -p \"" + output_dir + "\"";
#endif
    std::system(command.c_str());
}

}  // namespace

SLRParserABuilder::SLRParserABuilder(std::vector<Production> productions, std::string start_symbol)
    : input_productions_(std::move(productions)), start_symbol_(std::move(start_symbol)) {
    if (start_symbol_.empty()) {
        throw std::invalid_argument("start symbol must not be empty");
    }
}

void SLRParserABuilder::build() {
    resetBuiltState();

    all_productions_.clear();
    all_productions_.push_back({0, augmented_start_symbol_, {start_symbol_}});
    for (const auto& prod : input_productions_) {
        if (prod.id <= 0) {
            throw std::invalid_argument("production id must be > 0 for non-augmented grammar");
        }
        Production normalized = prod;
        if (normalized.rhs.size() == 1 && normalized.rhs[0] == kEpsilon) {
            normalized.rhs.clear();
        }
        all_productions_.push_back(std::move(normalized));
    }

    buildProductionIndex();
    computeSymbols();
    computeFirst();
    computeFollow();
    buildCanonicalCollection();
    buildTables();
}

void SLRParserABuilder::resetBuiltState() {
    terminals_.clear();
    nonterminals_.clear();
    first_.clear();
    follow_.clear();
    canonical_collection_.clear();
    state_transitions_.clear();
    action_table_.clear();
    goto_table_.clear();
    conflicts_.clear();
    prod_index_by_id_.clear();
    productions_by_lhs_.clear();
}

void SLRParserABuilder::buildProductionIndex() {
    for (std::size_t i = 0; i < all_productions_.size(); ++i) {
        const auto& prod = all_productions_[i];
        if (prod_index_by_id_.count(prod.id) != 0) {
            throw std::runtime_error("duplicate production id: " + std::to_string(prod.id));
        }
        prod_index_by_id_[prod.id] = i;
        productions_by_lhs_[prod.lhs].push_back(prod.id);
    }
}

void SLRParserABuilder::computeSymbols() {
    for (const auto& prod : all_productions_) {
        nonterminals_.insert(prod.lhs);
    }

    for (const auto& prod : all_productions_) {
        for (const auto& sym : prod.rhs) {
            if (sym == kEpsilon) {
                continue;
            }
            if (nonterminals_.count(sym) == 0) {
                terminals_.insert(sym);
            }
        }
    }

    terminals_.insert(kEndMarker);
}

void SLRParserABuilder::computeFirst() {
    for (const auto& terminal : terminals_) {
        first_[terminal].insert(terminal);
    }
    first_[kEpsilon].insert(kEpsilon);
    for (const auto& nonterminal : nonterminals_) {
        first_[nonterminal];
    }

    bool changed = true;
    while (changed) {
        changed = false;
        for (const auto& prod : all_productions_) {
            auto& first_lhs = first_[prod.lhs];
            const std::size_t before = first_lhs.size();

            if (prod.rhs.empty()) {
                first_lhs.insert(kEpsilon);
            } else {
                bool all_nullable = true;
                for (const auto& sym : prod.rhs) {
                    const auto& first_sym = first_[sym];
                    for (const auto& f : first_sym) {
                        if (f != kEpsilon) {
                            first_lhs.insert(f);
                        }
                    }
                    if (!ContainsEpsilon(first_sym)) {
                        all_nullable = false;
                        break;
                    }
                }
                if (all_nullable) {
                    first_lhs.insert(kEpsilon);
                }
            }

            if (first_lhs.size() != before) {
                changed = true;
            }
        }
    }
}

std::set<std::string> SLRParserABuilder::firstOfSequence(const std::vector<std::string>& sequence) const {
    if (sequence.empty()) {
        return {kEpsilon};
    }

    std::set<std::string> result;
    bool all_nullable = true;

    for (const auto& sym : sequence) {
        auto it = first_.find(sym);
        if (it == first_.end()) {
            result.insert(sym);
            all_nullable = false;
            break;
        }

        for (const auto& f : it->second) {
            if (f != kEpsilon) {
                result.insert(f);
            }
        }

        if (!ContainsEpsilon(it->second)) {
            all_nullable = false;
            break;
        }
    }

    if (all_nullable) {
        result.insert(kEpsilon);
    }
    return result;
}

void SLRParserABuilder::computeFollow() {
    for (const auto& nt : nonterminals_) {
        follow_[nt];
    }
    follow_[start_symbol_].insert(kEndMarker);

    bool changed = true;
    while (changed) {
        changed = false;

        for (const auto& prod : all_productions_) {
            const auto& rhs = prod.rhs;
            for (std::size_t i = 0; i < rhs.size(); ++i) {
                const std::string& B = rhs[i];
                if (!isNonterminal(B)) {
                    continue;
                }

                std::vector<std::string> beta(rhs.begin() + static_cast<std::ptrdiff_t>(i + 1), rhs.end());
                std::set<std::string> first_beta = firstOfSequence(beta);
                const std::size_t before = follow_[B].size();

                for (const auto& symbol : first_beta) {
                    if (symbol != kEpsilon) {
                        follow_[B].insert(symbol);
                    }
                }

                if (beta.empty() || first_beta.count(kEpsilon) != 0) {
                    const auto& follow_lhs = follow_[prod.lhs];
                    follow_[B].insert(follow_lhs.begin(), follow_lhs.end());
                }

                if (follow_[B].size() != before) {
                    changed = true;
                }
            }
        }
    }
}

std::set<LR0Item> SLRParserABuilder::closure(std::set<LR0Item> items) const {
    bool changed = true;
    while (changed) {
        changed = false;
        std::set<LR0Item> expanded = items;

        for (const auto& item : items) {
            const auto& prod = productionById(item.prod_id);
            if (item.dot_pos >= static_cast<int>(prod.rhs.size())) {
                continue;
            }

            const std::string& symbol = prod.rhs[item.dot_pos];
            if (!isNonterminal(symbol)) {
                continue;
            }

            auto it = productions_by_lhs_.find(symbol);
            if (it == productions_by_lhs_.end()) {
                continue;
            }
            for (const int next_prod_id : it->second) {
                LR0Item next_item{next_prod_id, 0};
                if (expanded.insert(next_item).second) {
                    changed = true;
                }
            }
        }

        items = std::move(expanded);
    }

    return items;
}

std::set<LR0Item> SLRParserABuilder::gotoState(const std::set<LR0Item>& items, const std::string& symbol) const {
    std::set<LR0Item> moved;
    for (const auto& item : items) {
        const auto& prod = productionById(item.prod_id);
        if (item.dot_pos < static_cast<int>(prod.rhs.size()) && prod.rhs[item.dot_pos] == symbol) {
            moved.insert({item.prod_id, item.dot_pos + 1});
        }
    }

    if (moved.empty()) {
        return {};
    }
    return closure(std::move(moved));
}

void SLRParserABuilder::buildCanonicalCollection() {
    canonical_collection_.clear();
    state_transitions_.clear();

    canonical_collection_.push_back(closure({LR0Item{0, 0}}));

    for (std::size_t i = 0; i < canonical_collection_.size(); ++i) {
        std::set<std::string> next_symbols;
        for (const auto& item : canonical_collection_[i]) {
            const auto& prod = productionById(item.prod_id);
            if (item.dot_pos < static_cast<int>(prod.rhs.size())) {
                next_symbols.insert(prod.rhs[item.dot_pos]);
            }
        }

        for (const auto& symbol : next_symbols) {
            auto next_state_items = gotoState(canonical_collection_[i], symbol);
            if (next_state_items.empty()) {
                continue;
            }

            int target_state = -1;
            for (std::size_t s = 0; s < canonical_collection_.size(); ++s) {
                if (canonical_collection_[s] == next_state_items) {
                    target_state = static_cast<int>(s);
                    break;
                }
            }

            if (target_state == -1) {
                target_state = static_cast<int>(canonical_collection_.size());
                canonical_collection_.push_back(std::move(next_state_items));
            }

            state_transitions_[{static_cast<int>(i), symbol}] = target_state;
        }
    }
}

void SLRParserABuilder::setAction(int state, const std::string& symbol, const ActionEntry& incoming) {
    const auto key = std::make_pair(state, symbol);
    auto it = action_table_.find(key);
    if (it == action_table_.end()) {
        action_table_[key] = incoming;
        return;
    }

    ActionEntry existing = it->second;
    if (existing.type == incoming.type && existing.target == incoming.target) {
        return;
    }

    ActionEntry chosen = existing;
    std::string resolution = "keep existing action";

    if (existing.type == ActionType::Accept) {
        chosen = existing;
        resolution = "keep accept";
    } else if (incoming.type == ActionType::Accept) {
        chosen = incoming;
        resolution = "prefer accept";
    } else if (existing.type == ActionType::Shift && incoming.type == ActionType::Reduce) {
        chosen = existing;
        resolution = "shift/reduce: prefer shift";
    } else if (existing.type == ActionType::Reduce && incoming.type == ActionType::Shift) {
        chosen = incoming;
        resolution = "shift/reduce: prefer shift";
    } else if (existing.type == ActionType::Reduce && incoming.type == ActionType::Reduce) {
        chosen = (incoming.target < existing.target) ? incoming : existing;
        resolution = "reduce/reduce: choose smaller production id";
    }

    it->second = chosen;
    conflicts_.push_back({state, symbol, existing, incoming, chosen, resolution});
}

void SLRParserABuilder::buildTables() {
    action_table_.clear();
    goto_table_.clear();

    for (std::size_t state = 0; state < canonical_collection_.size(); ++state) {
        for (const auto& item : canonical_collection_[state]) {
            const auto& prod = productionById(item.prod_id);

            if (item.dot_pos < static_cast<int>(prod.rhs.size())) {
                const std::string& symbol = prod.rhs[item.dot_pos];
                auto trans_it = state_transitions_.find({static_cast<int>(state), symbol});
                if (trans_it == state_transitions_.end()) {
                    continue;
                }

                if (isTerminal(symbol)) {
                    setAction(static_cast<int>(state), symbol, {ActionType::Shift, trans_it->second});
                } else if (isNonterminal(symbol)) {
                    goto_table_[{static_cast<int>(state), symbol}] = trans_it->second;
                }
            } else {
                if (prod.id == 0) {
                    setAction(static_cast<int>(state), kEndMarker, {ActionType::Accept, 0});
                } else {
                    const auto& follow_set = follow_[prod.lhs];
                    for (const auto& terminal : follow_set) {
                        setAction(static_cast<int>(state), terminal, {ActionType::Reduce, prod.id});
                    }
                }
            }
        }
    }
}

const Production& SLRParserABuilder::productionById(int id) const {
    auto it = prod_index_by_id_.find(id);
    if (it == prod_index_by_id_.end()) {
        throw std::runtime_error("unknown production id: " + std::to_string(id));
    }
    return all_productions_[it->second];
}

bool SLRParserABuilder::isTerminal(const std::string& symbol) const {
    return terminals_.count(symbol) != 0;
}

bool SLRParserABuilder::isNonterminal(const std::string& symbol) const {
    return nonterminals_.count(symbol) != 0;
}

std::vector<std::string> SLRParserABuilder::orderedTerminals() const {
    std::vector<std::string> symbols(terminals_.begin(), terminals_.end());
    std::sort(symbols.begin(), symbols.end(), [](const std::string& a, const std::string& b) {
        if (a == kEndMarker) {
            return false;
        }
        if (b == kEndMarker) {
            return true;
        }
        return a < b;
    });
    return symbols;
}

std::vector<std::string> SLRParserABuilder::orderedNonterminalsForGoto() const {
    std::vector<std::string> symbols;
    for (const auto& nt : nonterminals_) {
        if (nt != augmented_start_symbol_) {
            symbols.push_back(nt);
        }
    }
    std::sort(symbols.begin(), symbols.end());
    return symbols;
}

void SLRParserABuilder::dumpProductions(const std::string& path) const {
    std::ofstream out(path);
    if (!out.is_open()) {
        throw std::runtime_error("failed to open file: " + path);
    }

    out << "# Productions\n";
    for (const auto& prod : all_productions_) {
        const bool is_augmented = (prod.id == 0);
        out << "[" << prod.id << "] ";
        if (is_augmented) {
            out << "(augmented) ";
        }
        out << prod.lhs << " -> ";
        if (prod.rhs.empty()) {
            out << kEpsilon;
        } else {
            out << Join(prod.rhs, " ");
        }
        out << "\n";
    }
}

void SLRParserABuilder::dumpFirstFollow(const std::string& path) const {
    std::ofstream out(path);
    if (!out.is_open()) {
        throw std::runtime_error("failed to open file: " + path);
    }

    std::vector<std::string> nts;
    for (const auto& nt : nonterminals_) {
        if (nt != augmented_start_symbol_) {
            nts.push_back(nt);
        }
    }
    std::sort(nts.begin(), nts.end());

    out << "# FIRST / FOLLOW\n\n";
    for (const auto& nt : nts) {
        auto first_it = first_.find(nt);
        auto follow_it = follow_.find(nt);
        out << "FIRST(" << nt << ")  = "
            << SetToString(first_it == first_.end() ? std::set<std::string>{} : first_it->second) << "\n";
        out << "FOLLOW(" << nt << ") = "
            << SetToString(follow_it == follow_.end() ? std::set<std::string>{} : follow_it->second) << "\n\n";
    }
}

void SLRParserABuilder::dumpLR0Items(const std::string& path) const {
    std::ofstream out(path);
    if (!out.is_open()) {
        throw std::runtime_error("failed to open file: " + path);
    }

    out << "# Canonical LR(0) Item Sets\n\n";
    for (std::size_t i = 0; i < canonical_collection_.size(); ++i) {
        out << "I" << i << ":\n";
        for (const auto& item : canonical_collection_[i]) {
            const auto& prod = productionById(item.prod_id);
            out << "  [" << item.prod_id << "] " << prod.lhs << " -> ";
            for (int p = 0; p <= static_cast<int>(prod.rhs.size()); ++p) {
                if (p == item.dot_pos) {
                    out << ". ";
                }
                if (p < static_cast<int>(prod.rhs.size())) {
                    out << prod.rhs[p] << " ";
                }
            }
            if (prod.rhs.empty()) {
                out << ". ";
            }
            out << "\n";
        }

        std::vector<std::pair<std::string, int>> trans;
        for (const auto& kv : state_transitions_) {
            if (kv.first.first == static_cast<int>(i)) {
                trans.push_back({kv.first.second, kv.second});
            }
        }
        std::sort(trans.begin(), trans.end(), [](const auto& a, const auto& b) {
            return a.first < b.first;
        });
        if (!trans.empty()) {
            out << "  transitions:\n";
            for (const auto& t : trans) {
                out << "    goto(I" << i << ", " << t.first << ") = I" << t.second << "\n";
            }
        }

        out << "\n";
    }
}

void SLRParserABuilder::dumpActionCsv(const std::string& path) const {
    std::ofstream out(path);
    if (!out.is_open()) {
        throw std::runtime_error("failed to open file: " + path);
    }

    const auto terminals = orderedTerminals();
    out << "state";
    for (const auto& t : terminals) {
        out << "," << t;
    }
    out << "\n";

    for (std::size_t state = 0; state < canonical_collection_.size(); ++state) {
        out << state;
        for (const auto& t : terminals) {
            auto it = action_table_.find({static_cast<int>(state), t});
            if (it == action_table_.end()) {
                out << ",err";
            } else {
                out << "," << ActionEntryToString(it->second);
            }
        }
        out << "\n";
    }
}

void SLRParserABuilder::dumpGotoCsv(const std::string& path) const {
    std::ofstream out(path);
    if (!out.is_open()) {
        throw std::runtime_error("failed to open file: " + path);
    }

    const auto nonterminals = orderedNonterminalsForGoto();
    out << "state";
    for (const auto& nt : nonterminals) {
        out << "," << nt;
    }
    out << "\n";

    for (std::size_t state = 0; state < canonical_collection_.size(); ++state) {
        out << state;
        for (const auto& nt : nonterminals) {
            auto it = goto_table_.find({static_cast<int>(state), nt});
            if (it == goto_table_.end()) {
                out << ",";
            } else {
                out << "," << it->second;
            }
        }
        out << "\n";
    }
}

void SLRParserABuilder::dumpAll(const std::string& output_dir) const {
    EnsureDirectory(output_dir);
    dumpProductions(output_dir + "/productions.txt");
    dumpFirstFollow(output_dir + "/first_follow.txt");
    dumpLR0Items(output_dir + "/lr0_items.txt");
    dumpActionCsv(output_dir + "/action_table.csv");
    dumpGotoCsv(output_dir + "/goto_table.csv");

    std::ofstream conflict_out(output_dir + "/conflicts.txt");
    if (conflict_out.is_open()) {
        conflict_out << "# ACTION Table Conflicts\n";
        if (conflicts_.empty()) {
            conflict_out << "No conflicts.\n";
        } else {
            for (const auto& c : conflicts_) {
                conflict_out << "state=" << c.state
                             << ", symbol=" << c.symbol
                             << ", existing=" << ActionEntryToString(c.existing)
                             << ", incoming=" << ActionEntryToString(c.incoming)
                             << ", chosen=" << ActionEntryToString(c.chosen)
                             << ", resolution=" << c.resolution << "\n";
            }
        }
    }
}

std::string ActionEntryToString(const ActionEntry& action) {
    switch (action.type) {
        case ActionType::Shift:
            return "s" + std::to_string(action.target);
        case ActionType::Reduce:
            return "r" + std::to_string(action.target);
        case ActionType::Accept:
            return "acc";
        default:
            return "err";
    }
}

std::string TokenToGrammarTerminal(const Token& token) {
    switch (token.type) {
        case TokenType::KW:
            // Assignment grammar uses Ident for function/variable names.
            // Lexer may classify "main" as KW, so remap it to Ident here.
            if (ToLowerAscii(token.lexeme) == "main") {
                return "Ident";
            }
            return ToLowerAscii(token.lexeme);
        case TokenType::OP:
        case TokenType::SE:
            return token.lexeme;
        case TokenType::IDN:
            return "Ident";
        case TokenType::INT:
            return "IntConst";
        case TokenType::FLOAT:
            return "floatConst";
        case TokenType::EOF_TOK:
            return kEndMarker;
        default:
            return "ERROR";
    }
}

std::vector<Production> BuildDefaultCMinusGrammar() {
    std::vector<Production> g;
    int id = 1;

    g.push_back({id++, "Program", {"compUnit"}});
    g.push_back({id++, "compUnit", {"compUnit", "element"}});
    g.push_back({id++, "compUnit", {"element"}});
    g.push_back({id++, "element", {"decl"}});
    g.push_back({id++, "element", {"funcDef"}});

    g.push_back({id++, "decl", {"constDecl"}});
    g.push_back({id++, "decl", {"varDecl"}});

    g.push_back({id++, "constDecl", {"const", "bType", "constDefList", ";"}});
    g.push_back({id++, "constDefList", {"constDefList", ",", "constDef"}});
    g.push_back({id++, "constDefList", {"constDef"}});

    g.push_back({id++, "bType", {"int"}});
    g.push_back({id++, "bType", {"float"}});

    g.push_back({id++, "constDef", {"Ident", "=", "constInitVal"}});
    g.push_back({id++, "constInitVal", {"constExp"}});

    g.push_back({id++, "varDecl", {"bType", "varDefList", ";"}});
    g.push_back({id++, "varDefList", {"varDefList", ",", "varDef"}});
    g.push_back({id++, "varDefList", {"varDef"}});

    g.push_back({id++, "varDef", {"Ident"}});
    g.push_back({id++, "varDef", {"Ident", "=", "initVal"}});
    g.push_back({id++, "initVal", {"exp"}});

    g.push_back({id++, "funcDef", {"funcType", "Ident", "(", ")", "block"}});
    g.push_back({id++, "funcDef", {"bType", "Ident", "(", ")", "block"}});
    g.push_back({id++, "funcDef", {"funcType", "Ident", "(", "funcFParams", ")", "block"}});
    g.push_back({id++, "funcDef", {"bType", "Ident", "(", "funcFParams", ")", "block"}});

    g.push_back({id++, "funcType", {"void"}});
    g.push_back({id++, "funcFParams", {"funcFParams", ",", "funcFParam"}});
    g.push_back({id++, "funcFParams", {"funcFParam"}});
    g.push_back({id++, "funcFParam", {"bType", "Ident"}});

    g.push_back({id++, "block", {"{", "blockItemList", "}"}});
    g.push_back({id++, "block", {"{", "}"}});
    g.push_back({id++, "blockItemList", {"blockItemList", "blockItem"}});
    g.push_back({id++, "blockItemList", {"blockItem"}});
    g.push_back({id++, "blockItem", {"decl"}});
    g.push_back({id++, "blockItem", {"stmt"}});

    g.push_back({id++, "stmt", {"lVal", "=", "exp", ";"}});
    g.push_back({id++, "stmt", {"exp", ";"}});
    g.push_back({id++, "stmt", {";"}});
    g.push_back({id++, "stmt", {"block"}});
    g.push_back({id++, "stmt", {"if", "(", "cond", ")", "stmt", "ElsePart"}});
    g.push_back({id++, "stmt", {"return", "exp", ";"}});
    g.push_back({id++, "stmt", {"return", ";"}});

    g.push_back({id++, "ElsePart", {"else", "stmt"}});
    g.push_back({id++, "ElsePart", {}});

    g.push_back({id++, "lVal", {"Ident"}});
    g.push_back({id++, "exp", {"lOrExp"}});
    g.push_back({id++, "lOrExp", {"lAndExp"}});
    g.push_back({id++, "lOrExp", {"lOrExp", "||", "lAndExp"}});
    g.push_back({id++, "lAndExp", {"eqExp"}});
    g.push_back({id++, "lAndExp", {"lAndExp", "&&", "eqExp"}});
    g.push_back({id++, "eqExp", {"relExp"}});
    g.push_back({id++, "eqExp", {"eqExp", "==", "relExp"}});
    g.push_back({id++, "eqExp", {"eqExp", "!=", "relExp"}});
    g.push_back({id++, "relExp", {"addExp"}});
    g.push_back({id++, "relExp", {"relExp", "<", "addExp"}});
    g.push_back({id++, "relExp", {"relExp", ">", "addExp"}});
    g.push_back({id++, "relExp", {"relExp", "<=", "addExp"}});
    g.push_back({id++, "relExp", {"relExp", ">=", "addExp"}});
    g.push_back({id++, "addExp", {"mulExp"}});
    g.push_back({id++, "addExp", {"addExp", "+", "mulExp"}});
    g.push_back({id++, "addExp", {"addExp", "-", "mulExp"}});
    g.push_back({id++, "mulExp", {"unaryExp"}});
    g.push_back({id++, "mulExp", {"mulExp", "*", "unaryExp"}});
    g.push_back({id++, "mulExp", {"mulExp", "/", "unaryExp"}});
    g.push_back({id++, "mulExp", {"mulExp", "%", "unaryExp"}});
    g.push_back({id++, "unaryExp", {"primaryExp"}});
    g.push_back({id++, "unaryExp", {"unaryOp", "unaryExp"}});
    g.push_back({id++, "unaryExp", {"Ident", "(", ")"}});
    g.push_back({id++, "unaryExp", {"Ident", "(", "funcRParams", ")"}});
    g.push_back({id++, "primaryExp", {"(", "exp", ")"}});
    g.push_back({id++, "primaryExp", {"lVal"}});
    g.push_back({id++, "primaryExp", {"number"}});
    g.push_back({id++, "number", {"IntConst"}});
    g.push_back({id++, "number", {"floatConst"}});
    g.push_back({id++, "unaryOp", {"+"}});
    g.push_back({id++, "unaryOp", {"-"}});
    g.push_back({id++, "unaryOp", {"!"}});
    g.push_back({id++, "funcRParams", {"exp", ",", "funcRParams"}});
    g.push_back({id++, "funcRParams", {"exp"}});
    g.push_back({id++, "constExp", {"addExp"}});
    g.push_back({id++, "cond", {"lOrExp"}});

    return g;
}

}  // namespace parser_a
