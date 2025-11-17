#include "../include/semantic.h"
#include <iostream>

SemanticAnalyzer::SemanticAnalyzer() {
    pushScope(); // escopo global
}

void SemanticAnalyzer::pushScope() {
    variableScopes.emplace_back();
}

void SemanticAnalyzer::popScope() {
    if (!variableScopes.empty()) variableScopes.pop_back();
}

void SemanticAnalyzer::declareVariable(const std::string &name) {
    variableScopes.back()[name] = true;
}

bool SemanticAnalyzer::isVariableDeclared(const std::string &name) const {
    for (auto it = variableScopes.rbegin(); it != variableScopes.rend(); ++it) {
        if (it->count(name)) return true;
    }
    return false;
}

void SemanticAnalyzer::registerFunction(const FuncDeclNode *func) {
    if (functions.count(func->name)) {
        throw SemanticError("função '" + func->name + "' já declarada.");
    }

    functions[func->name] = FunctionInfo{
        func,
        static_cast<int>(func->params.size())
    };
}

bool SemanticAnalyzer::isFunctionDeclared(const std::string &name) const {
    return functions.count(name) > 0;
}

int SemanticAnalyzer::getFunctionParamCount(const std::string &name) const {
    auto it = functions.find(name);
    if (it == functions.end()) return -1;
    return it->second.paramCount;
}

void SemanticAnalyzer::analyzeNode(const NodePtr &node) {
    if (!node) return;

    if (auto n = dynamic_cast<const AssignNode*>(node.get())) {
        analyzeAssign(n);

    } else if (auto n = dynamic_cast<const FuncDeclNode*>(node.get())) {
        analyzeFuncDecl(n);

    } else if (auto n = dynamic_cast<const BinaryOpNode*>(node.get())) {
        analyzeBinary(n);

    } else if (auto n = dynamic_cast<const VarNode*>(node.get())) {
        analyzeVar(n);

    } else if (auto n = dynamic_cast<const FuncCallNode*>(node.get())) {
        analyzeFuncCall(n);

    } else if (dynamic_cast<const NumberNode*>(node.get())) {
        return; // número não causa nada

    } else {
        // futuras extensões
    }
}

void SemanticAnalyzer::analyzeAssign(const AssignNode *n) {
    analyzeNode(n->expr);
    declareVariable(n->name);
}

void SemanticAnalyzer::analyzeFuncDecl(const FuncDeclNode *n) {
    pushScope();

    std::unordered_map<std::string, bool> seen;

    for (const std::string &p : n->params) {
        if (seen.count(p)) {
            throw SemanticError(
                "parâmetro duplicado '" + p + "' na função '" + n->name + "'"
            );
        }
        seen[p] = true;
        declareVariable(p);
    }

    analyzeNode(n->body);

    popScope();
}

void SemanticAnalyzer::analyzeBinary(const BinaryOpNode *n) {
    if (n->left) analyzeNode(n->left);
    if (n->right) analyzeNode(n->right);
}

void SemanticAnalyzer::analyzeVar(const VarNode *n) {
    if (!isVariableDeclared(n->name)) {
        throw SemanticError("variável '" + n->name + "' não declarada.");
    }
}

void SemanticAnalyzer::analyzeFuncCall(const FuncCallNode *n) {
    if (!isFunctionDeclared(n->name)) {
        throw SemanticError("função '" + n->name + "' não declarada.");
    }

    int expected = getFunctionParamCount(n->name);
    int received = static_cast<int>(n->args.size());

    if (expected != received) {
        throw SemanticError(
            "função '" + n->name + "' esperava " +
            std::to_string(expected) + " argumentos, recebeu " +
            std::to_string(received) + "."
        );
    }

    for (const auto &arg : n->args)
        analyzeNode(arg);
}

void SemanticAnalyzer::analyze(const std::vector<NodePtr> &ast) {
    for (const auto &node : ast) {
        if (auto f = dynamic_cast<const FuncDeclNode*>(node.get())) {
            registerFunction(f);
        }
    }

    for (const auto &node : ast) {
        analyzeNode(node);
    }
}

