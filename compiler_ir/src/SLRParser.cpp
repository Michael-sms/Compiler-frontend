#include "SLRParser.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
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

            const std::string& symbol = prod.rhs[static_cast<std::size_t>(item.dot_pos)];
            if (!isNonterminal(symbol)) {
                continue;
            }

            auto it = productions_by_lhs_.find(symbol);
            if (it == productions_by_lhs_.end()) {
                continue;
            }

            for (int prod_id : it->second) {
                LR0Item candidate{prod_id, 0};
                if (expanded.insert(candidate).second) {
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
        if (item.dot_pos < static_cast<int>(prod.rhs.size()) && prod.rhs[static_cast<std::size_t>(item.dot_pos)] == symbol) {
            moved.insert({item.prod_id, item.dot_pos + 1});
        }
    }
    if (moved.empty()) {
        return moved;
    }
    return closure(std::move(moved));
}

void SLRParserABuilder::buildCanonicalCollection() {
    canonical_collection_.clear();
    state_transitions_.clear();

    std::set<LR0Item> start_items;
    start_items.insert({0, 0});
    start_items = closure(std::move(start_items));

    canonical_collection_.push_back(start_items);

    bool changed = true;
    while (changed) {
        changed = false;
        for (std::size_t i = 0; i < canonical_collection_.size(); ++i) {
            auto items = canonical_collection_[i];

            std::set<std::string> symbols;
            for (const auto& item : items) {
                const auto& prod = productionById(item.prod_id);
                if (item.dot_pos < static_cast<int>(prod.rhs.size())) {
                    symbols.insert(prod.rhs[static_cast<std::size_t>(item.dot_pos)]);
                }
            }

            for (const auto& sym : symbols) {
                std::set<LR0Item> goto_items = gotoState(items, sym);
                if (goto_items.empty()) {
                    continue;
                }

                auto it = std::find(canonical_collection_.begin(), canonical_collection_.end(), goto_items);
                int next_state = -1;
                if (it == canonical_collection_.end()) {
                    canonical_collection_.push_back(goto_items);
                    next_state = static_cast<int>(canonical_collection_.size()) - 1;
                    changed = true;
                } else {
                    next_state = static_cast<int>(std::distance(canonical_collection_.begin(), it));
                }

                state_transitions_[{static_cast<int>(i), sym}] = next_state;
            }
        }
    }
}

void SLRParserABuilder::setAction(int state, const std::string& symbol, const ActionEntry& incoming) {
    auto key = std::make_pair(state, symbol);
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
    std::string resolution = "kept existing";
    if (existing.type == ActionType::Error && incoming.type != ActionType::Error) {
        chosen = incoming;
        resolution = "replace error";
    }

    conflicts_.push_back({state, symbol, existing, incoming, chosen, resolution});
    it->second = chosen;
}

void SLRParserABuilder::buildTables() {
    action_table_.clear();
    goto_table_.clear();

    for (std::size_t state = 0; state < canonical_collection_.size(); ++state) {
        const auto& items = canonical_collection_[state];

        for (const auto& item : items) {
            const auto& prod = productionById(item.prod_id);
            if (item.dot_pos < static_cast<int>(prod.rhs.size())) {
                std::string sym = prod.rhs[static_cast<std::size_t>(item.dot_pos)];
                auto trans_it = state_transitions_.find({static_cast<int>(state), sym});
                if (trans_it != state_transitions_.end()) {
                    if (isTerminal(sym)) {
                        setAction(static_cast<int>(state), sym, {ActionType::Shift, trans_it->second});
                    } else {
                        goto_table_[{static_cast<int>(state), sym}] = trans_it->second;
                    }
                }
                continue;
            }

            if (prod.lhs == augmented_start_symbol_) {
                setAction(static_cast<int>(state), kEndMarker, {ActionType::Accept, 0});
                continue;
            }

            for (const auto& follow_sym : follow_[prod.lhs]) {
                setAction(static_cast<int>(state), follow_sym, {ActionType::Reduce, prod.id});
            }
        }
    }
}

const Production& SLRParserABuilder::productionById(int id) const {
    auto it = prod_index_by_id_.find(id);
    if (it == prod_index_by_id_.end()) {
        throw std::runtime_error("production id not found: " + std::to_string(id));
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
    std::vector<std::string> ordered(terminals_.begin(), terminals_.end());
    std::sort(ordered.begin(), ordered.end());
    return ordered;
}

std::vector<std::string> SLRParserABuilder::orderedNonterminalsForGoto() const {
    std::vector<std::string> ordered(nonterminals_.begin(), nonterminals_.end());
    std::sort(ordered.begin(), ordered.end());
    return ordered;
}

void SLRParserABuilder::dumpProductions(const std::string& path) const {
    std::ofstream out(path);
    if (!out.is_open()) {
        throw std::runtime_error("failed to open " + path);
    }
    for (const auto& prod : all_productions_) {
        out << prod.id << "\t" << prod.lhs << " -> ";
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
        throw std::runtime_error("failed to open " + path);
    }

    out << "FIRST sets:\n";
    for (const auto& nt : orderedNonterminalsForGoto()) {
        out << nt << " : " << SetToString(first_.at(nt)) << "\n";
    }
    out << "\nFOLLOW sets:\n";
    for (const auto& nt : orderedNonterminalsForGoto()) {
        out << nt << " : " << SetToString(follow_.at(nt)) << "\n";
    }
}

void SLRParserABuilder::dumpLR0Items(const std::string& path) const {
    std::ofstream out(path);
    if (!out.is_open()) {
        throw std::runtime_error("failed to open " + path);
    }

    for (std::size_t i = 0; i < canonical_collection_.size(); ++i) {
        out << "I" << i << ":\n";
        for (const auto& item : canonical_collection_[i]) {
            const auto& prod = productionById(item.prod_id);
            out << "  [" << prod.lhs << " -> ";
            for (std::size_t j = 0; j <= prod.rhs.size(); ++j) {
                if (j == static_cast<std::size_t>(item.dot_pos)) {
                    out << "· ";
                }
                if (j < prod.rhs.size()) {
                    out << prod.rhs[j] << " ";
                }
            }
            out << "]\n";
        }
        out << "\n";
    }
}

void SLRParserABuilder::dumpActionCsv(const std::string& path) const {
    std::ofstream out(path);
    if (!out.is_open()) {
        throw std::runtime_error("failed to open " + path);
    }

    auto terminals = orderedTerminals();
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
                out << ",";
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
        throw std::runtime_error("failed to open " + path);
    }

    auto nonterminals = orderedNonterminalsForGoto();
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
    dumpActionCsv(output_dir + "/action.csv");
    dumpGotoCsv(output_dir + "/goto.csv");
}

std::vector<Production> BuildDefaultCMinusGrammar() {
    std::vector<Production> prods;
    int id = 1;

    prods.push_back({id++, "Program", {"compUnit"}});
    prods.push_back({id++, "compUnit", {"decl", "compUnit"}});
    prods.push_back({id++, "compUnit", {"funcDef", "compUnit"}});
    prods.push_back({id++, "compUnit", {kEpsilon}});
    prods.push_back({id++, "decl", {"constDecl"}});
    prods.push_back({id++, "decl", {"varDecl"}});
    prods.push_back({id++, "constDecl", {"const", "bType", "constDef", ",", "constDecl"}});
    prods.push_back({id++, "constDecl", {"const", "bType", "constDef", ";"}});
    prods.push_back({id++, "bType", {"int"}});
    prods.push_back({id++, "bType", {"float"}});
    prods.push_back({id++, "constDef", {"Ident", "=", "constInitVal"}});
    prods.push_back({id++, "constInitVal", {"constExp"}});
    prods.push_back({id++, "varDecl", {"bType", "varDef", ",", "varDecl"}});
    prods.push_back({id++, "varDecl", {"bType", "varDef", ";"}});
    prods.push_back({id++, "varDef", {"Ident"}});
    prods.push_back({id++, "varDef", {"Ident", "=", "initVal"}});
    prods.push_back({id++, "initVal", {"exp"}});
    prods.push_back({id++, "funcDef", {"funcType", "FuncName", "(", "funcFParams", ")", "block"}});
    prods.push_back({id++, "funcDef", {"funcType", "FuncName", "(", ")", "block"}});
    prods.push_back({id++, "FuncName", {"Ident"}});
    prods.push_back({id++, "FuncName", {"main"}});  // main 关键字作为函数名
    prods.push_back({id++, "funcType", {"void"}});
    prods.push_back({id++, "funcType", {"int"}});
    prods.push_back({id++, "funcFParams", {"funcFParam", ",", "funcFParams"}});
    prods.push_back({id++, "funcFParams", {"funcFParam"}});
    prods.push_back({id++, "funcFParam", {"bType", "Ident"}});
    prods.push_back({id++, "block", {"{", "blockItemList", "}"}});
    prods.push_back({id++, "blockItemList", {"blockItem", "blockItemList"}});
    prods.push_back({id++, "blockItemList", {kEpsilon}});
    prods.push_back({id++, "blockItem", {"decl"}});
    prods.push_back({id++, "blockItem", {"stmt"}});
    prods.push_back({id++, "stmt", {"lVal", "=", "exp", ";"}});
    prods.push_back({id++, "stmt", {"exp", ";"}});
    prods.push_back({id++, "stmt", {";"}});
    prods.push_back({id++, "stmt", {"block"}});
    prods.push_back({id++, "stmt", {"if", "(", "cond", ")", "stmt", "else", "stmt"}});
    prods.push_back({id++, "stmt", {"if", "(", "cond", ")", "stmt"}});
    prods.push_back({id++, "stmt", {"return", "exp", ";"}});
    prods.push_back({id++, "stmt", {"return", ";"}});
    prods.push_back({id++, "exp", {"addExp"}});
    prods.push_back({id++, "cond", {"lOrExp"}});
    prods.push_back({id++, "lVal", {"Ident"}});
    prods.push_back({id++, "primaryExp", {"(", "exp", ")"}});
    prods.push_back({id++, "primaryExp", {"lVal"}});
    prods.push_back({id++, "primaryExp", {"number"}});
    prods.push_back({id++, "number", {"IntConst"}});
    prods.push_back({id++, "number", {"floatConst"}});
    prods.push_back({id++, "unaryExp", {"primaryExp"}});
    prods.push_back({id++, "unaryExp", {"Ident", "(", "funcRParams", ")"}});
    prods.push_back({id++, "unaryExp", {"Ident", "(", ")"}});
    prods.push_back({id++, "unaryExp", {"unaryOp", "unaryExp"}});
    prods.push_back({id++, "unaryOp", {"+"}});
    prods.push_back({id++, "unaryOp", {"-"}});
    prods.push_back({id++, "unaryOp", {"!"}});
    prods.push_back({id++, "funcRParams", {"funcRParam", ",", "funcRParams"}});
    prods.push_back({id++, "funcRParams", {"funcRParam"}});
    prods.push_back({id++, "funcRParam", {"exp"}});
    prods.push_back({id++, "mulExp", {"unaryExp"}});
    prods.push_back({id++, "mulExp", {"mulExp", "*", "unaryExp"}});
    prods.push_back({id++, "mulExp", {"mulExp", "/", "unaryExp"}});
    prods.push_back({id++, "mulExp", {"mulExp", "%", "unaryExp"}});
    prods.push_back({id++, "addExp", {"mulExp"}});
    prods.push_back({id++, "addExp", {"addExp", "+", "mulExp"}});
    prods.push_back({id++, "addExp", {"addExp", "-", "mulExp"}});
    prods.push_back({id++, "relExp", {"addExp"}});
    prods.push_back({id++, "relExp", {"relExp", "<", "addExp"}});
    prods.push_back({id++, "relExp", {"relExp", ">", "addExp"}});
    prods.push_back({id++, "relExp", {"relExp", "<=", "addExp"}});
    prods.push_back({id++, "relExp", {"relExp", ">=", "addExp"}});
    prods.push_back({id++, "eqExp", {"relExp"}});
    prods.push_back({id++, "eqExp", {"eqExp", "==", "relExp"}});
    prods.push_back({id++, "eqExp", {"eqExp", "!=", "relExp"}});
    prods.push_back({id++, "lAndExp", {"eqExp"}});
    prods.push_back({id++, "lAndExp", {"lAndExp", "&&", "eqExp"}});
    prods.push_back({id++, "lOrExp", {"lAndExp"}});
    prods.push_back({id++, "lOrExp", {"lOrExp", "||", "lAndExp"}});
    prods.push_back({id++, "constExp", {"addExp"}});
    return prods;
}

std::string TokenToGrammarTerminal(const Token& token) {
    switch (token.type) {
        case TokenType::KW:
            return ToLowerAscii(token.lexeme);
        case TokenType::IDN:
            return "Ident";
        case TokenType::INT:
            return "IntConst";
        case TokenType::FLOAT:
            return "floatConst";
        case TokenType::OP:
            return token.lexeme;
        case TokenType::SE:
            return token.lexeme;
        case TokenType::EOF_TOK:
            return kEndMarker;
        default:
            return token.lexeme;
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
            return "";
    }
}

}  // namespace parser_a

namespace parser_b {
namespace {
std::string ActionToTraceString(parser_a::ActionType type) {
    switch (type) {
        case parser_a::ActionType::Shift:
            return "move";
        case parser_a::ActionType::Reduce:
            return "reduction";
        case parser_a::ActionType::Accept:
            return "accept";
        default:
            return "error";
    }
}

std::string TerminalFromToken(const Token& token) {
    return parser_a::TokenToGrammarTerminal(token);
}

std::string BuildErrorMessage(const Token& token, const std::string& terminal) {
    std::ostringstream oss;
    oss << "Syntax error at line " << token.line << ", column " << token.column
        << ": unexpected '" << token.lexeme << "' (" << terminal << ")";
    return oss.str();
}
}  // namespace

SLRParserB::SLRParserB(std::vector<parser_a::Production> productions, std::string startSymbol)
    : builder_(std::move(productions), std::move(startSymbol)) {
    std::cout << "SLRParserB build start.\n";
    std::cout.flush();
    builder_.build();
    std::cout << "SLRParserB build done.\n";
    for (const auto& prod : builder_.productions()) {
        productionsById_[prod.id] = prod;
    }
}

SLRParserB SLRParserB::BuildDefault() {
    return SLRParserB(parser_a::BuildDefaultCMinusGrammar(), "Program");
}

const parser_a::Production* SLRParserB::findProduction(int id) const {
    auto it = productionsById_.find(id);
    if (it == productionsById_.end()) {
        return nullptr;
    }
    return &it->second;
}

ParseResult SLRParserB::parse(const std::vector<Token>& tokens, bool trace, std::ostream& out) const {
    ParseResult result;
    if (tokens.empty()) {
        result.errors.push_back("Syntax error: empty token stream");
        return result;
    }

    std::vector<int> stateStack;
    std::vector<std::string> symbolStack;
    std::vector<frontend::ASTNodePtr> nodeStack;

    stateStack.push_back(0);

    std::size_t index = 0;
    int step = 1;

    while (true) {
        const Token& lookahead = (index < tokens.size()) ? tokens[index] : tokens.back();
        std::string terminal = TerminalFromToken(lookahead);
        int state = stateStack.back();

        auto actionIt = builder_.actionTable().find({state, terminal});
        parser_a::ActionEntry action;
        if (actionIt != builder_.actionTable().end()) {
            action = actionIt->second;
        }

        std::string topSymbol = symbolStack.empty() ? parser_a::kEndMarker : symbolStack.back();
        if (trace) {
            out << step << "\t" << topSymbol << "#" << terminal << "\t" << ActionToTraceString(action.type)
                << "\n";
        }
        ++step;

        if (lookahead.type == TokenType::ERROR) {
            result.errors.push_back(BuildErrorMessage(lookahead, terminal));
            return result;
        }

        switch (action.type) {
            case parser_a::ActionType::Shift: {
                symbolStack.push_back(terminal);
                stateStack.push_back(action.target);
                if (terminal != parser_a::kEndMarker) {
                    nodeStack.push_back(frontend::MakeTerminalNode(terminal, lookahead));
                }
                ++index;
                break;
            }
            case parser_a::ActionType::Reduce: {
                const parser_a::Production* prod = findProduction(action.target);
                if (prod == nullptr) {
                    result.errors.push_back("Syntax error: unknown production id " + std::to_string(action.target));
                    return result;
                }

                result.reductions.push_back(prod->id);

                std::vector<frontend::ASTNodePtr> children;
                const std::size_t popCount = prod->rhs.size();
                if (popCount > symbolStack.size() || popCount > stateStack.size() - 1) {
                    result.errors.push_back("Syntax error: reduction stack underflow");
                    return result;
                }

                for (std::size_t i = 0; i < popCount; ++i) {
                    symbolStack.pop_back();
                    stateStack.pop_back();
                    if (!nodeStack.empty()) {
                        children.push_back(nodeStack.back());
                        nodeStack.pop_back();
                    }
                }

                std::reverse(children.begin(), children.end());

                const Token* anchorToken = nullptr;
                if (children.empty()) {
                    anchorToken = &lookahead;
                }
                auto node = frontend::MakeNonterminalNode(prod->lhs, std::move(children), anchorToken);

                symbolStack.push_back(prod->lhs);
                nodeStack.push_back(node);

                int gotoState = -1;
                auto gotoIt = builder_.gotoTable().find({stateStack.back(), prod->lhs});
                if (gotoIt == builder_.gotoTable().end()) {
                    result.errors.push_back("Syntax error: missing goto for state " + std::to_string(stateStack.back()) +
                                             ", symbol " + prod->lhs);
                    return result;
                }
                gotoState = gotoIt->second;
                stateStack.push_back(gotoState);
                break;
            }
            case parser_a::ActionType::Accept:
                result.accepted = true;
                if (!nodeStack.empty()) {
                    result.root = nodeStack.back();
                }
                return result;
            default:
                result.errors.push_back(BuildErrorMessage(lookahead, terminal));
                return result;
        }
    }
}

void SLRParserB::PrintReductionSequence(const std::vector<int>& reductions, std::ostream& out) {
    out << "Reduction sequence (production ids):";
    if (reductions.empty()) {
        out << " <empty>\n";
        return;
    }
    out << "\n";
    for (std::size_t i = 0; i < reductions.size(); ++i) {
        out << reductions[i];
        if (i + 1 < reductions.size()) {
            out << ' ';
        }
    }
    out << "\n";
}

void SLRParserB::PrintErrors(const std::vector<std::string>& errors, std::ostream& out) {
    if (errors.empty()) {
        out << "Errors: <none>\n";
        return;
    }
    out << "Errors:\n";
    for (const auto& err : errors) {
        out << "- " << err << "\n";
    }
}

}  // namespace parser_b

