#include "IRGenerator.h"

#include <algorithm>
#include <sstream>

#include "BasicBlock.h"
#include "Constant.h"
#include "Function.h"
#include "GlobalVariable.h"
#include "IRbuilder.h"
#include "Module.h"
#include "Type.h"

namespace {
constexpr const char* kEpsilon = "epsilon";

bool IsEpsilon(const frontend::ASTNodePtr& node) {
    return node && node->nodeType == kEpsilon;
}

frontend::ASTNodePtr FindFirstChild(const frontend::ASTNodePtr& node, const std::string& type) {
    if (!node) {
        return nullptr;
    }
    for (const auto& child : node->children) {
        if (child && child->nodeType == type) {
            return child;
        }
    }
    return nullptr;
}

std::vector<frontend::ASTNodePtr> FindChildren(const frontend::ASTNodePtr& node, const std::string& type) {
    std::vector<frontend::ASTNodePtr> result;
    if (!node) {
        return result;
    }
    for (const auto& child : node->children) {
        if (child && child->nodeType == type) {
            result.push_back(child);
        }
    }
    return result;
}

void CollectFuncFParams(const frontend::ASTNodePtr& node, std::vector<frontend::ASTNodePtr>& out) {
    if (!node) {
        return;
    }
    if (node->nodeType == "funcFParam") {
        out.push_back(node);
        return;
    }
    if (node->nodeType == "funcFParams") {
        auto first = FindFirstChild(node, "funcFParam");
        if (first) {
            out.push_back(first);
        }
        auto rest = FindFirstChild(node, "funcFParams");
        if (rest) {
            CollectFuncFParams(rest, out);
        }
    }
}

void CollectFuncRParams(const frontend::ASTNodePtr& node, std::vector<frontend::ASTNodePtr>& out) {
    if (!node) {
        return;
    }
    if (node->nodeType == "funcRParam") {
        out.push_back(node);
        return;
    }
    if (node->nodeType == "funcRParams") {
        auto first = FindFirstChild(node, "funcRParam");
        if (first) {
            out.push_back(first);
        }
        auto rest = FindFirstChild(node, "funcRParams");
        if (rest) {
            CollectFuncRParams(rest, out);
        }
    }
}

void CollectBlockItems(const frontend::ASTNodePtr& node, std::vector<frontend::ASTNodePtr>& out) {
    if (!node) {
        return;
    }
    if (node->nodeType == "blockItem") {
        out.push_back(node);
        return;
    }
    if (node->nodeType == "blockItemList") {
        auto first = FindFirstChild(node, "blockItem");
        if (first) {
            out.push_back(first);
        }
        auto rest = FindFirstChild(node, "blockItemList");
        if (rest) {
            CollectBlockItems(rest, out);
        }
    }
}



std::string ExtractTokenLexeme(const frontend::ASTNodePtr& node) {
    if (!node) {
        return {};
    }
    if (node->hasToken) {
        return node->token.lexeme;
    }
    for (const auto& child : node->children) {
        if (child && child->hasToken) {
            return child->token.lexeme;
        }
    }
    return {};
}

std::string ExtractFuncName(const frontend::ASTNodePtr& node) {
    if (!node) {
        return {};
    }
    if (node->nodeType == "FuncName") {
        auto ident = FindFirstChild(node, "Ident");
        if (ident) {
            return ExtractTokenLexeme(ident);
        }
        auto mainKw = FindFirstChild(node, "main");
        if (mainKw) {
            return "main";
        }
    }
    if (node->nodeType == "Ident") {
        return ExtractTokenLexeme(node);
    }
    auto ident = FindFirstChild(node, "Ident");
    if (ident) {
        return ExtractTokenLexeme(ident);
    }
    auto mainKw = FindFirstChild(node, "main");
    if (mainKw) {
        return "main";
    }
    return {};
}

int ParseIntFromNode(const frontend::ASTNodePtr& node, bool& ok) {
    ok = false;
    if (!node) {
        return 0;
    }
    if (node->nodeType == "IntConst" && node->hasToken) {
        ok = true;
        return std::stoi(node->token.lexeme);
    }
    for (const auto& child : node->children) {
        if (child && child->nodeType == "IntConst" && child->hasToken) {
            ok = true;
            return std::stoi(child->token.lexeme);
        }
    }
    return 0;
}
}  // namespace

IRGenerator::IRGenerator(const std::string& moduleName)
    : module_(new Module(moduleName)),
      builder_(new IRBuilder(nullptr, module_)),
      currentFunction_(nullptr),
      currentBB_(nullptr) {
    enterScope();
}

IRGenerator::~IRGenerator() {
    delete builder_;
}

void IRGenerator::generate(const frontend::ASTNodePtr& root) {
    if (!root) {
        return;
    }
    declareRuntimeFunctions();
    visitProgram(root);
    module_->set_print_name();
}

std::string IRGenerator::print() const {
    return module_->print();
}

void IRGenerator::addError(const frontend::ASTNodePtr& node, const std::string& message) {
    std::ostringstream oss;
    if (node && node->line > 0 && node->column > 0) {
        oss << "IR error at " << node->line << ":" << node->column << ": ";
    } else {
        oss << "IR error: ";
    }
    oss << message;
    errors_.push_back(oss.str());
}

void IRGenerator::enterScope() {
    scopes_.push_back(Scope{});
}

void IRGenerator::exitScope() {
    if (!scopes_.empty()) {
        scopes_.pop_back();
    }
}

bool IRGenerator::insertVar(const std::string& name, Value* addr) {
    if (scopes_.empty()) {
        enterScope();
    }
    auto& scope = scopes_.back().vars;
    if (scope.find(name) != scope.end()) {
        return false;
    }
    scope[name] = addr;
    return true;
}

Value* IRGenerator::lookupVar(const std::string& name) const {
    for (auto it = scopes_.rbegin(); it != scopes_.rend(); ++it) {
        auto found = it->vars.find(name);
        if (found != it->vars.end()) {
            return found->second;
        }
    }
    return nullptr;
}

Type* IRGenerator::resolveBType(const frontend::ASTNodePtr& bTypeNode) {
    if (!bTypeNode) {
        return module_->get_int32_type();
    }
    if (FindFirstChild(bTypeNode, "int") || bTypeNode->nodeType == "int") {
        return module_->get_int32_type();
    }
    if (FindFirstChild(bTypeNode, "float") || bTypeNode->nodeType == "float") {
        return module_->get_int32_type();
    }
    return module_->get_int32_type();
}

std::string IRGenerator::extractIdent(const frontend::ASTNodePtr& node) const {
    if (!node) {
        return {};
    }
    if (node->nodeType == "Ident") {
        return ExtractTokenLexeme(node);
    }
    auto ident = FindFirstChild(node, "Ident");
    return ExtractTokenLexeme(ident);
}

void IRGenerator::visitProgram(const frontend::ASTNodePtr& node) {
    if (!node) {
        return;
    }
    if (node->nodeType == "Program") {
        visitCompUnit(FindFirstChild(node, "compUnit"));
        return;
    }
    visitCompUnit(node);
}

void IRGenerator::visitCompUnit(const frontend::ASTNodePtr& node) {
    if (!node || IsEpsilon(node)) {
        return;
    }
    if (node->nodeType != "compUnit") {
        visitDecl(node);
        visitFuncDef(node);
        return;
    }
    if (node->children.empty()) {
        return;
    }
    if (node->children.size() == 1 && IsEpsilon(node->children[0])) {
        return;
    }
    if (node->children.size() >= 2) {
        auto first = node->children[0];
        auto rest = node->children[1];
        if (first && first->nodeType == "decl") {
            visitDecl(first);
        } else if (first && first->nodeType == "funcDef") {
            visitFuncDef(first);
        }
        visitCompUnit(rest);
    }
}

void IRGenerator::visitDecl(const frontend::ASTNodePtr& node) {
    if (!node) {
        return;
    }
    if (node->nodeType == "decl") {
        auto constDecl = FindFirstChild(node, "constDecl");
        if (constDecl) {
            visitConstDecl(constDecl);
            return;
        }
        auto varDecl = FindFirstChild(node, "varDecl");
        if (varDecl) {
            visitVarDecl(varDecl);
            return;
        }
    } else if (node->nodeType == "constDecl") {
        visitConstDecl(node);
    } else if (node->nodeType == "varDecl") {
        visitVarDecl(node);
    }
}

void IRGenerator::visitConstDecl(const frontend::ASTNodePtr& node) {
    if (!node || node->nodeType != "constDecl") {
        return;
    }
    auto bTypeNode = FindFirstChild(node, "bType");
    Type* type = resolveBType(bTypeNode);
    auto constDef = FindFirstChild(node, "constDef");
    if (constDef) {
        auto name = extractIdent(constDef);
        if (!scopes_.empty() && scopes_.back().vars.find(name) != scopes_.back().vars.end()) {
            addError(constDef, "duplicate const definition: " + name);
            return;
        }
        bool ok = false;
        int initVal = 0;
        auto initNode = FindFirstChild(constDef, "constInitVal");
        if (initNode) {
            initVal = evalConstInt(initNode, ok);
            if (!ok) {
                addError(initNode, "const init must be integer literal expression");
                initVal = 0;
            }
        }

        if (!currentFunction_) {
            auto init = ConstantInt::get(initVal, module_);
            auto gv = GlobalVariable::create(name, module_, type, true, init);
            insertVar(name, gv);
        } else {
            auto alloca = builder_->create_alloca(type);
            builder_->create_store(ConstantInt::get(initVal, module_), alloca);
            insertVar(name, alloca);
        }
    }

    auto nested = FindFirstChild(node, "constDecl");
    if (nested) {
        visitConstDecl(nested);
    }
}

void IRGenerator::visitVarDecl(const frontend::ASTNodePtr& node) {
    if (!node || node->nodeType != "varDecl") {
        return;
    }
    auto bTypeNode = FindFirstChild(node, "bType");
    Type* type = resolveBType(bTypeNode);
    auto varDef = FindFirstChild(node, "varDef");
    if (varDef) {
        auto name = extractIdent(varDef);
        if (!scopes_.empty() && scopes_.back().vars.find(name) != scopes_.back().vars.end()) {
            addError(varDef, "duplicate variable definition: " + name);
            return;
        }
        auto initNode = FindFirstChild(varDef, "initVal");
        if (!currentFunction_) {
            int initVal = 0;
            auto init = ConstantInt::get(initVal, module_);
            auto gv = GlobalVariable::create(name, module_, type, false, init);
            insertVar(name, gv);
        } else {
            auto alloca = builder_->create_alloca(type);
            insertVar(name, alloca);
            if (initNode) {
                auto expNode = FindFirstChild(initNode, "exp");
                auto val = visitExp(expNode ? expNode : initNode);
                if (val) {
                    builder_->create_store(val, alloca);
                }
            }
        }
    }

    auto nested = FindFirstChild(node, "varDecl");
    if (nested) {
        visitVarDecl(nested);
    }
}

void IRGenerator::visitFuncDef(const frontend::ASTNodePtr& node) {
    if (!node || node->nodeType != "funcDef") {
        return;
    }
    auto funcTypeNode = FindFirstChild(node, "funcType");
    Type* retType = resolveBType(funcTypeNode);
    if (FindFirstChild(funcTypeNode, "void") || (funcTypeNode && funcTypeNode->nodeType == "void")) {
        retType = module_->get_void_type();
    }

    auto funcNameNode = FindFirstChild(node, "FuncName");
    std::string funcName = ExtractFuncName(funcNameNode);
    if (funcName.empty()) {
        auto identNode = FindFirstChild(node, "Ident");
        funcName = ExtractFuncName(identNode);
    }
    std::vector<Type*> paramTypes;
    std::vector<std::string> paramNames;

    auto paramsNode = FindFirstChild(node, "funcFParams");
    if (paramsNode) {
        std::vector<frontend::ASTNodePtr> paramNodes;
        CollectFuncFParams(paramsNode, paramNodes);
        for (const auto& param : paramNodes) {
            auto bType = FindFirstChild(param, "bType");
            paramTypes.push_back(resolveBType(bType));
            paramNames.push_back(extractIdent(param));
        }
    }

    auto funcType = FunctionType::get(retType, paramTypes);
    auto func = Function::create(funcType, funcName, module_);
    currentFunction_ = func;
    builder_->set_curFunc(func);

    auto entry = BasicBlock::create(module_, "", func);
    if (!funcName.empty()) {
        entry->set_name(funcName + "_ENTRY");
    }
    currentBB_ = entry;
    builder_->set_insert_point(entry);

    enterScope();

    auto argIt = func->arg_begin();
    for (std::size_t i = 0; i < paramNames.size(); ++i) {
        auto alloca = builder_->create_alloca(paramTypes[i]);
        builder_->create_store(*argIt, alloca);
        insertVar(paramNames[i], alloca);
        ++argIt;
    }

    auto blockNode = FindFirstChild(node, "block");
    if (blockNode) {
        visitBlock(blockNode);
    }

    if (currentBB_ && !currentBB_->get_terminator()) {
        if (retType->is_void_type()) {
            builder_->create_void_ret();
        } else {
            builder_->create_ret(ConstantInt::get(0, module_));
        }
    }

    exitScope();
    currentFunction_ = nullptr;
    currentBB_ = nullptr;
}

void IRGenerator::declareRuntimeFunctions() {
    std::vector<std::pair<std::string, FunctionType*>> decls;
    auto i32 = module_->get_int32_type();
    auto i32Ptr = module_->get_int32_ptr_type();
    auto voidTy = module_->get_void_type();

    decls.push_back({"getint", FunctionType::get(i32, {})});
    decls.push_back({"getch", FunctionType::get(i32, {})});
    decls.push_back({"getarray", FunctionType::get(i32, {i32Ptr})});
    decls.push_back({"putint", FunctionType::get(voidTy, {i32})});
    decls.push_back({"putch", FunctionType::get(voidTy, {i32})});
    decls.push_back({"putarray", FunctionType::get(voidTy, {i32, i32Ptr})});
    decls.push_back({"starttime", FunctionType::get(voidTy, {})});
    decls.push_back({"stoptime", FunctionType::get(voidTy, {})});

    for (const auto& decl : decls) {
        Function::create(decl.second, decl.first, module_);
    }
}

void IRGenerator::visitBlock(const frontend::ASTNodePtr& node) {
    if (!node || node->nodeType != "block") {
        return;
    }
    enterScope();
    auto listNode = FindFirstChild(node, "blockItemList");
    if (listNode) {
        std::vector<frontend::ASTNodePtr> items;
        CollectBlockItems(listNode, items);
        for (const auto& item : items) {
            visitBlockItem(item);
        }
    }
    exitScope();
}

void IRGenerator::visitBlockItem(const frontend::ASTNodePtr& node) {
    if (!node || node->nodeType != "blockItem") {
        return;
    }
    auto decl = FindFirstChild(node, "decl");
    if (decl) {
        visitDecl(decl);
        return;
    }
    auto stmt = FindFirstChild(node, "stmt");
    if (stmt) {
        visitStmt(stmt);
    }
}

void IRGenerator::visitStmt(const frontend::ASTNodePtr& node) {
    if (!node || node->nodeType != "stmt") {
        return;
    }
    if (FindFirstChild(node, "if")) {
        visitIfStmt(node);
        return;
    }
    if (FindFirstChild(node, "return")) {
        visitReturnStmt(node);
        return;
    }
    if (FindFirstChild(node, "block")) {
        visitBlock(FindFirstChild(node, "block"));
        return;
    }
    if (FindFirstChild(node, "lVal")) {
        visitAssignStmt(node);
        return;
    }
    auto expNode = FindFirstChild(node, "exp");
    if (expNode) {
        visitExp(expNode);
    }
}

void IRGenerator::visitAssignStmt(const frontend::ASTNodePtr& node) {
    auto lValNode = FindFirstChild(node, "lVal");
    auto expNode = FindFirstChild(node, "exp");
    if (!lValNode || !expNode) {
        return;
    }
    auto lhsVal = visitLVal(lValNode, true);
    auto rhsVal = visitExp(expNode);
    if (lhsVal && rhsVal) {
        builder_->create_store(lhsVal, rhsVal);
    }
}

void IRGenerator::visitReturnStmt(const frontend::ASTNodePtr& node) {
    auto expNode = FindFirstChild(node, "exp");
    if (!currentFunction_) {
        return;
    }
    auto retType = currentFunction_->get_return_type();
    if (retType->is_void_type()) {
        builder_->create_void_ret();
        return;
    }
    if (expNode) {
        auto val = visitExp(expNode);
        if (val) {
            builder_->create_ret(val);
            return;
        }
    }
    builder_->create_ret(ConstantInt::get(0, module_));
}

void IRGenerator::visitIfStmt(const frontend::ASTNodePtr& node) {
    auto condNode = FindFirstChild(node, "cond");
    auto stmtNodes = FindChildren(node, "stmt");
    if (!condNode || stmtNodes.empty()) {
        return;
    }

    Value* condVal = visitCond(condNode);
    condVal = ensureBool(condVal);
    if (!condVal) {
        return;
    }

    BasicBlock* thenBB = BasicBlock::create(module_, "then", currentFunction_);
    BasicBlock* mergeBB = BasicBlock::create(module_, "ifcont", currentFunction_);
    BasicBlock* elseBB = nullptr;
    if (FindFirstChild(node, "else") && stmtNodes.size() >= 2) {
        elseBB = BasicBlock::create(module_, "else", currentFunction_);
    }

    if (elseBB) {
        builder_->create_cond_br(condVal, thenBB, elseBB);
    } else {
        builder_->create_cond_br(condVal, thenBB, mergeBB);
    }

    currentBB_ = thenBB;
    builder_->set_insert_point(thenBB);
    visitStmt(stmtNodes[0]);
    if (!currentBB_->get_terminator()) {
        builder_->create_br(mergeBB);
    }

    if (elseBB) {
        currentBB_ = elseBB;
        builder_->set_insert_point(elseBB);
        visitStmt(stmtNodes[1]);
        if (!currentBB_->get_terminator()) {
            builder_->create_br(mergeBB);
        }
    }

    currentBB_ = mergeBB;
    builder_->set_insert_point(mergeBB);
}

Value* IRGenerator::visitExp(const frontend::ASTNodePtr& node) {
    if (!node) {
        return nullptr;
    }
    if (node->nodeType == "exp") {
        return visitAddExp(FindFirstChild(node, "addExp"));
    }
    if (node->nodeType == "addExp") {
        return visitAddExp(node);
    }
    if (node->nodeType == "mulExp") {
        return visitMulExp(node);
    }
    if (node->nodeType == "unaryExp") {
        return visitUnaryExp(node);
    }
    if (node->nodeType == "primaryExp") {
        return visitPrimaryExp(node);
    }
    if (node->nodeType == "lVal") {
        return visitLVal(node, true);
    }
    if (node->nodeType == "number") {
        return visitNumber(node);
    }
    if (node->nodeType == "relExp") {
        return visitRelExp(node);
    }
    if (node->nodeType == "eqExp") {
        return visitEqExp(node);
    }
    if (node->nodeType == "lAndExp") {
        return visitLAndExp(node);
    }
    if (node->nodeType == "lOrExp") {
        return visitLOrExp(node);
    }
    return nullptr;
}

Value* IRGenerator::visitAddExp(const frontend::ASTNodePtr& node) {
    if (!node) {
        return nullptr;
    }
    if (node->children.size() == 1) {
        return visitMulExp(FindFirstChild(node, "mulExp"));
    }
    if (node->children.size() >= 3) {
        auto lhs = visitAddExp(node->children[0]);
        auto rhs = visitMulExp(node->children[2]);
        if (!lhs || !rhs) {
            return nullptr;
        }
        lhs = ensureInt32(lhs);
        rhs = ensureInt32(rhs);
        const std::string op = node->children[1]->nodeType;
        if (op == "+") {
            return builder_->create_iadd(lhs, rhs);
        }
        if (op == "-") {
            return builder_->create_isub(lhs, rhs);
        }
    }
    return nullptr;
}

Value* IRGenerator::visitMulExp(const frontend::ASTNodePtr& node) {
    if (!node) {
        return nullptr;
    }
    if (node->children.size() == 1) {
        return visitUnaryExp(FindFirstChild(node, "unaryExp"));
    }
    if (node->children.size() >= 3) {
        auto lhs = visitMulExp(node->children[0]);
        auto rhs = visitUnaryExp(node->children[2]);
        if (!lhs || !rhs) {
            return nullptr;
        }
        lhs = ensureInt32(lhs);
        rhs = ensureInt32(rhs);
        const std::string op = node->children[1]->nodeType;
        if (op == "*") {
            return builder_->create_imul(lhs, rhs);
        }
        if (op == "/") {
            return builder_->create_isdiv(lhs, rhs);
        }
        if (op == "%") {
            return builder_->create_irem(lhs, rhs);
        }
    }
    return nullptr;
}

Value* IRGenerator::visitUnaryExp(const frontend::ASTNodePtr& node) {
    if (!node) {
        return nullptr;
    }
    if (node->children.size() == 1) {
        auto primary = FindFirstChild(node, "primaryExp");
        if (primary) {
            return visitPrimaryExp(primary);
        }
    }

    if (node->children.size() >= 2 && node->children[0]->nodeType == "unaryOp") {
        auto opNode = node->children[0];
        auto target = visitUnaryExp(node->children[1]);
        if (!target) {
            return nullptr;
        }
        target = ensureInt32(target);
        auto opToken = FindFirstChild(opNode, "+");
        if (opToken) {
            return target;
        }
        opToken = FindFirstChild(opNode, "-");
        if (opToken) {
            auto zero = ConstantInt::get(0, module_);
            return builder_->create_isub(zero, target);
        }
        opToken = FindFirstChild(opNode, "!");
        if (opToken) {
            return builder_->create_icmp_eq(target, ConstantInt::get(0, module_));
        }
    }

    if (node->children.size() >= 3 && node->children[0]->nodeType == "Ident") {
        std::string funcName = ExtractTokenLexeme(node->children[0]);
        std::vector<Value*> args;
        auto paramsNode = FindFirstChild(node, "funcRParams");
        if (paramsNode) {
            std::vector<frontend::ASTNodePtr> paramNodes;
            CollectFuncRParams(paramsNode, paramNodes);
            for (const auto& param : paramNodes) {
                auto expNode = FindFirstChild(param, "exp");
                auto val = visitExp(expNode);
                if (val) {
                    args.push_back(ensureInt32(val));
                }
            }
        }

        Function* callee = nullptr;
        for (auto func : module_->get_functions()) {
            if (func->get_name() == funcName) {
                callee = func;
                break;
            }
        }
        if (!callee) {
            addError(node, "unknown function: " + funcName);
            return ConstantInt::get(0, module_);
        }
        return builder_->create_call(callee, args);
    }

    return nullptr;
}

Value* IRGenerator::visitPrimaryExp(const frontend::ASTNodePtr& node) {
    if (!node) {
        return nullptr;
    }
    auto expNode = FindFirstChild(node, "exp");
    if (expNode) {
        return visitExp(expNode);
    }
    auto lVal = FindFirstChild(node, "lVal");
    if (lVal) {
        return visitLVal(lVal, true);
    }
    auto number = FindFirstChild(node, "number");
    if (number) {
        return visitNumber(number);
    }
    return nullptr;
}

Value* IRGenerator::visitLVal(const frontend::ASTNodePtr& node, bool load) {
    auto name = extractIdent(node);
    if (name.empty()) {
        return nullptr;
    }
    auto addr = lookupVar(name);
    if (!addr) {
        addError(node, "undefined variable: " + name);
        return nullptr;
    }
    if (!load) {
        return addr;
    }
    if (!addr->get_type()->is_pointer_type()) {
        return addr;
    }
    auto elemTy = addr->get_type()->get_pointer_element_type();
    return builder_->create_load(elemTy, addr);
}

Value* IRGenerator::visitNumber(const frontend::ASTNodePtr& node) {
    bool ok = false;
    int value = ParseIntFromNode(node, ok);
    if (!ok) {
        std::string literal = ExtractTokenLexeme(node);
        if (!literal.empty()) {
            try {
                float fval = std::stof(literal);
                return ConstantInt::get(static_cast<int>(fval), module_);
            } catch (const std::exception&) {
                return ConstantInt::get(0, module_);
            }
        }
        return ConstantInt::get(0, module_);
    }
    return ConstantInt::get(value, module_);
}

Value* IRGenerator::visitRelExp(const frontend::ASTNodePtr& node) {
    if (!node) {
        return nullptr;
    }
    if (node->children.size() == 1) {
        return visitAddExp(FindFirstChild(node, "addExp"));
    }
    if (node->children.size() >= 3) {
        auto lhs = ensureInt32(visitRelExp(node->children[0]));
        auto rhs = ensureInt32(visitAddExp(node->children[2]));
        if (!lhs || !rhs) {
            return nullptr;
        }
        const std::string op = node->children[1]->nodeType;
        if (op == "<") {
            return builder_->create_icmp_lt(lhs, rhs);
        }
        if (op == ">") {
            return builder_->create_icmp_gt(lhs, rhs);
        }
        if (op == "<=") {
            return builder_->create_icmp_le(lhs, rhs);
        }
        if (op == ">=") {
            return builder_->create_icmp_ge(lhs, rhs);
        }
    }
    return nullptr;
}

Value* IRGenerator::visitEqExp(const frontend::ASTNodePtr& node) {
    if (!node) {
        return nullptr;
    }
    if (node->children.size() == 1) {
        return visitRelExp(FindFirstChild(node, "relExp"));
    }
    if (node->children.size() >= 3) {
        auto lhs = ensureInt32(visitEqExp(node->children[0]));
        auto rhs = ensureInt32(visitRelExp(node->children[2]));
        if (!lhs || !rhs) {
            return nullptr;
        }
        const std::string op = node->children[1]->nodeType;
        if (op == "==") {
            return builder_->create_icmp_eq(lhs, rhs);
        }
        if (op == "!=") {
            return builder_->create_icmp_ne(lhs, rhs);
        }
    }
    return nullptr;
}

Value* IRGenerator::visitLAndExp(const frontend::ASTNodePtr& node) {
    if (!node) {
        return nullptr;
    }
    if (node->children.size() == 1) {
        return visitEqExp(FindFirstChild(node, "eqExp"));
    }
    if (node->children.size() >= 3) {
        auto lhs = ensureBool(visitLAndExp(node->children[0]));
        auto rhs = ensureBool(visitEqExp(node->children[2]));
        if (!lhs || !rhs) {
            return nullptr;
        }
        auto lhsI32 = ensureInt32(lhs);
        auto rhsI32 = ensureInt32(rhs);
        auto mul = builder_->create_imul(lhsI32, rhsI32);
        return builder_->create_icmp_ne(mul, ConstantInt::get(0, module_));
    }
    return nullptr;
}

Value* IRGenerator::visitLOrExp(const frontend::ASTNodePtr& node) {
    if (!node) {
        return nullptr;
    }
    if (node->children.size() == 1) {
        return visitLAndExp(FindFirstChild(node, "lAndExp"));
    }
    if (node->children.size() >= 3) {
        auto lhs = ensureBool(visitLOrExp(node->children[0]));
        auto rhs = ensureBool(visitLAndExp(node->children[2]));
        if (!lhs || !rhs) {
            return nullptr;
        }
        auto lhsI32 = ensureInt32(lhs);
        auto rhsI32 = ensureInt32(rhs);
        auto sum = builder_->create_iadd(lhsI32, rhsI32);
        return builder_->create_icmp_ne(sum, ConstantInt::get(0, module_));
    }
    return nullptr;
}

Value* IRGenerator::visitCond(const frontend::ASTNodePtr& node) {
    if (!node) {
        return nullptr;
    }
    auto lOr = FindFirstChild(node, "lOrExp");
    if (lOr) {
        return visitLOrExp(lOr);
    }
    return nullptr;
}

Value* IRGenerator::ensureInt32(Value* value) {
    if (!value) {
        return nullptr;
    }
    if (value->get_type()->is_int32_type()) {
        return value;
    }
    if (value->get_type()->is_int1_type()) {
        return builder_->create_zext(value, module_->get_int32_type());
    }
    return value;
}

Value* IRGenerator::ensureBool(Value* value) {
    if (!value) {
        return nullptr;
    }
    if (value->get_type()->is_int1_type()) {
        return value;
    }
    if (value->get_type()->is_int32_type()) {
        return builder_->create_icmp_ne(value, ConstantInt::get(0, module_));
    }
    return value;
}

int IRGenerator::evalConstInt(const frontend::ASTNodePtr& node, bool& ok) {
    ok = false;
    if (!node) {
        return 0;
    }
    if (node->nodeType == "constInitVal") {
        return evalConstInt(FindFirstChild(node, "constExp"), ok);
    }
    if (node->nodeType == "initVal") {
        auto expNode = FindFirstChild(node, "exp");
        if (expNode) {
            return evalConstInt(expNode, ok);
        }
    }
    if (node->nodeType == "constExp") {
        return evalConstInt(FindFirstChild(node, "addExp"), ok);
    }
    if (node->nodeType == "exp") {
        return evalConstInt(FindFirstChild(node, "addExp"), ok);
    }
    if (node->nodeType == "addExp") {
        if (node->children.size() == 1) {
            return evalConstInt(FindFirstChild(node, "mulExp"), ok);
        }
        bool okL = false;
        bool okR = false;
        int lhs = evalConstInt(node->children[0], okL);
        int rhs = evalConstInt(node->children[2], okR);
        if (!okL || !okR) {
            return 0;
        }
        ok = true;
        if (node->children[1]->nodeType == "+") {
            return lhs + rhs;
        }
        if (node->children[1]->nodeType == "-") {
            return lhs - rhs;
        }
    }
    if (node->nodeType == "mulExp") {
        if (node->children.size() == 1) {
            return evalConstInt(FindFirstChild(node, "unaryExp"), ok);
        }
        bool okL = false;
        bool okR = false;
        int lhs = evalConstInt(node->children[0], okL);
        int rhs = evalConstInt(node->children[2], okR);
        if (!okL || !okR) {
            return 0;
        }
        ok = true;
        if (node->children[1]->nodeType == "*") {
            return lhs * rhs;
        }
        if (node->children[1]->nodeType == "/") {
            return rhs == 0 ? 0 : lhs / rhs;
        }
        if (node->children[1]->nodeType == "%") {
            return rhs == 0 ? 0 : lhs % rhs;
        }
    }
    if (node->nodeType == "unaryExp") {
        if (node->children.size() == 1) {
            return evalConstInt(FindFirstChild(node, "primaryExp"), ok);
        }
        if (node->children.size() >= 2 && node->children[0]->nodeType == "unaryOp") {
            bool innerOk = false;
            int val = evalConstInt(node->children[1], innerOk);
            if (!innerOk) {
                return 0;
            }
            ok = true;
            if (FindFirstChild(node->children[0], "+")) {
                return val;
            }
            if (FindFirstChild(node->children[0], "-")) {
                return -val;
            }
            if (FindFirstChild(node->children[0], "!")) {
                return val == 0 ? 1 : 0;
            }
        }
    }
    if (node->nodeType == "primaryExp") {
        auto inner = FindFirstChild(node, "number");
        if (inner) {
            bool parseOk = false;
            int val = ParseIntFromNode(inner, parseOk);
            ok = parseOk;
            return val;
        }
        auto expNode = FindFirstChild(node, "exp");
        if (expNode) {
            return evalConstInt(expNode, ok);
        }
    }
    if (node->nodeType == "number") {
        bool parseOk = false;
        int val = ParseIntFromNode(node, parseOk);
        ok = parseOk;
        return val;
    }
    if (node->nodeType == "IntConst") {
        bool parseOk = false;
        int val = ParseIntFromNode(node, parseOk);
        ok = parseOk;
        return val;
    }
    return 0;
}


