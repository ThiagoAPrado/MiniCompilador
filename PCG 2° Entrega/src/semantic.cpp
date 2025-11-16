#include "../include/semantic.h"
#include <iostream>

SemanticAnalyzer::SemanticAnalyzer() {
    // iniciar com escopo global
    pushScope();
}

void SemanticAnalyzer::pushScope() {
    variableScopes.emplace_back();
}

void SemanticAnalyzer::popScope() {
    if (!variableScopes.empty()) variableScopes.pop_back();
}

void SemanticAnalyzer::declareVariable(const std::string &name) {
    if (variableScopes.empty()) pushScope();
    variableScopes.back()[name] = true;
}

bool SemanticAnalyzer::isVariableDeclared(const std::string &name) const {
    for (auto it = variableScopes.rbegin(); it != variableScopes.rend(); ++it) {
        if (it->count(name)) return true;
    }
    return false;
}

void SemanticAnalyzer::declareFunction(const std::string &name, int paramCount) {
    functions[name] = FunctionInfo{paramCount};
}

bool SemanticAnalyzer::isFunctionDeclared(const std::string &name) const {
    return functions.count(name) > 0;
}

int SemanticAnalyzer::getFunctionParamCount(const std::string &name) const {
    auto it = functions.find(name);
    if (it == functions.end()) return -1;
    return it->second.paramCount;
}

void SemanticAnalyzer::analyze(const std::vector<NodePtr> &ast) {
    for (const auto &node : ast) {
        analyzeNode(node);
    }
}

/* Dispatch para o tipo concreto do Node (usamos dynamic_cast) */
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
    } else if (auto n = dynamic_cast<const NumberNode*>(node.get())) {
        // nada a fazer
        (void)n;
    } else {
        // outros tipos que você pode adicionar depois
    }
}

void SemanticAnalyzer::analyzeAssign(const AssignNode *n) {
    // primeiro analisa a expressão do lado direito
    analyzeNode(n->expr);
    // depois declara a variável no escopo atual
    declareVariable(n->name);
}

void SemanticAnalyzer::analyzeFuncDecl(const FuncDeclNode *n) {
    // registrar a função (antes de analisar o corpo, para permitir chamadas recursivas)
    declareFunction(n->name, static_cast<int>(n->params.size()));

    // novo escopo para a função
    pushScope();

    // declarar parâmetros no escopo local
    for (const auto &p : n->params) declareVariable(p);

    // analisar o corpo (body)
    analyzeNode(n->body);

    // sair do escopo local
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
            "função '" + n->name + "' esperava " + std::to_string(expected) +
            " argumentos, recebeu " + std::to_string(received) + "."
        );
    }
    // verificar argumentos recursivamente
    for (const auto &arg : n->args) analyzeNode(arg);
}
