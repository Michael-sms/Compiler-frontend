#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "ast.h"

class BasicBlock;
class Function;
class IRBuilder;
class Module;
class Type;
class Value;

class IRGenerator {
public:
    explicit IRGenerator(const std::string& moduleName = "sysy_module");
    ~IRGenerator();

    void generate(const frontend::ASTNodePtr& root);
    std::string print() const;
    Module* module() const { return module_; }
    const std::vector<std::string>& errors() const { return errors_; }

private:
    struct Scope {
        std::unordered_map<std::string, Value*> vars;
    };

    Module* module_;
    IRBuilder* builder_;
    Function* currentFunction_;
    BasicBlock* currentBB_;
    std::vector<Scope> scopes_;
    std::vector<std::string> errors_;

    void addError(const frontend::ASTNodePtr& node, const std::string& message);

    void enterScope();
    void exitScope();
    bool insertVar(const std::string& name, Value* addr);
    Value* lookupVar(const std::string& name) const;

    Type* resolveBType(const frontend::ASTNodePtr& bTypeNode);
    std::string extractIdent(const frontend::ASTNodePtr& node) const;

    void visitProgram(const frontend::ASTNodePtr& node);
    void visitCompUnit(const frontend::ASTNodePtr& node);
    void visitDecl(const frontend::ASTNodePtr& node);
    void visitConstDecl(const frontend::ASTNodePtr& node);
    void visitVarDecl(const frontend::ASTNodePtr& node);
    void visitFuncDef(const frontend::ASTNodePtr& node);
    void visitBlock(const frontend::ASTNodePtr& node);
    void visitBlockItem(const frontend::ASTNodePtr& node);
    void visitStmt(const frontend::ASTNodePtr& node);
    void visitAssignStmt(const frontend::ASTNodePtr& node);
    void visitReturnStmt(const frontend::ASTNodePtr& node);
    void visitIfStmt(const frontend::ASTNodePtr& node);

    void declareRuntimeFunctions();

    Value* visitExp(const frontend::ASTNodePtr& node);
    Value* visitAddExp(const frontend::ASTNodePtr& node);
    Value* visitMulExp(const frontend::ASTNodePtr& node);
    Value* visitUnaryExp(const frontend::ASTNodePtr& node);
    Value* visitPrimaryExp(const frontend::ASTNodePtr& node);
    Value* visitLVal(const frontend::ASTNodePtr& node, bool load);
    Value* visitNumber(const frontend::ASTNodePtr& node);
    Value* visitRelExp(const frontend::ASTNodePtr& node);
    Value* visitEqExp(const frontend::ASTNodePtr& node);
    Value* visitLAndExp(const frontend::ASTNodePtr& node);
    Value* visitLOrExp(const frontend::ASTNodePtr& node);
    Value* visitCond(const frontend::ASTNodePtr& node);

    Value* ensureInt32(Value* value);
    Value* ensureBool(Value* value);
    int evalConstInt(const frontend::ASTNodePtr& node, bool& ok);
};
