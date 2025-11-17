#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "ast.h"
#include <string>
#include <unordered_map>
#include <vector>
#include <stdexcept>

class SemanticError : public std::runtime_error {
public:
    explicit SemanticError(const std::string &msg)
        : std::runtime_error("Erro semântico: " + msg) {}
};

struct FunctionInfo {
    const FuncDeclNode* decl;  
    int paramCount;
};

class SemanticAnalyzer {
private:
    std::vector<std::unordered_map<std::string, bool>> variableScopes;
    std::unordered_map<std::string, FunctionInfo> functions;

    void pushScope();
    void popScope();

    void declareVariable(const std::string &name);
    bool isVariableDeclared(const std::string &name) const;

    void registerFunction(const FuncDeclNode *func);
    bool isFunctionDeclared(const std::string &name) const;
    int getFunctionParamCount(const std::string &name) const;

    void analyzeNode(const NodePtr &node);
    void analyzeAssign(const AssignNode *n);
    void analyzeFuncDecl(const FuncDeclNode *n);
    void analyzeBinary(const BinaryOpNode *n);
    void analyzeVar(const VarNode *n);
    void analyzeFuncCall(const FuncCallNode *n);

public:
    SemanticAnalyzer();
    void analyze(const std::vector<NodePtr> &ast);
};

#endif // SEMANTIC_H
