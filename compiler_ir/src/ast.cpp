#include "ast.h"

#include <utility>

namespace frontend {
namespace {
bool HasValidPosition(const ASTNodePtr& node) {
    return node && node->line > 0 && node->column > 0;
}

void ApplyPositionFromChildren(ASTNode& node) {
    for (const auto& child : node.children) {
        if (HasValidPosition(child)) {
            node.line = child->line;
            node.column = child->column;
            return;
        }
    }
}
}  // namespace

ASTNodePtr MakeTerminalNode(const std::string& nodeType, const Token& token) {
    auto node = std::make_shared<ASTNode>();
    node->nodeType = nodeType;
    node->hasToken = true;
    node->token = token;
    node->line = token.line;
    node->column = token.column;
    return node;
}

ASTNodePtr MakeNonterminalNode(const std::string& nodeType,
                               std::vector<ASTNodePtr> children,
                               const Token* anchorToken) {
    auto node = std::make_shared<ASTNode>();
    node->nodeType = nodeType;
    node->children = std::move(children);
    if (anchorToken != nullptr) {
        node->line = anchorToken->line;
        node->column = anchorToken->column;
    } else {
        ApplyPositionFromChildren(*node);
    }
    return node;
}

void DumpAstPreorder(const ASTNodePtr& node, std::ostream& out, int indent) {
    if (!node) {
        return;
    }

    for (int i = 0; i < indent; ++i) {
        out << ' ';
    }
    out << node->nodeType << " (" << node->line << ":" << node->column << ")";
    if (node->hasToken) {
        out << " [" << node->token.lexeme << "]";
    }
    out << "\n";

    for (const auto& child : node->children) {
        DumpAstPreorder(child, out, indent + 2);
    }
}

}  // namespace frontend
