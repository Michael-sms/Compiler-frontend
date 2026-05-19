#include "slr_parser_b.h"

#include <algorithm>
#include <sstream>
#include <utility>

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
    builder_.build();
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
