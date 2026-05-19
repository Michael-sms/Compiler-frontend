#pragma once

#include <map>
#include <ostream>
#include <string>
#include <vector>

#include "ast.h"
#include "slr_parser_a.h"

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
