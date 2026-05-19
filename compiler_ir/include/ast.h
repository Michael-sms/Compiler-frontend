#pragma once

#include <memory>
#include <ostream>
#include <string>
#include <vector>

#include "token.h"

namespace frontend {

struct ASTNode {
    std::string nodeType;
    std::vector<std::shared_ptr<ASTNode>> children;
    bool hasToken = false;
    Token token{};
    int line = 0;
    int column = 0;
};

using ASTNodePtr = std::shared_ptr<ASTNode>;

ASTNodePtr MakeTerminalNode(const std::string& nodeType, const Token& token);
ASTNodePtr MakeNonterminalNode(const std::string& nodeType,
                               std::vector<ASTNodePtr> children,
                               const Token* anchorToken);
void DumpAstPreorder(const ASTNodePtr& node, std::ostream& out, int indent = 0);

}  // namespace frontend
