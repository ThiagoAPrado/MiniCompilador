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

struct VariableInfo {
    Type type;
    bool initialized;
};

struct FunctionInfo {
    const FuncDeclNode* decl;  
    int paramCount;
    Type returnType;
};

class SemanticAnalyzer {
private:
    std::vector<std::unordered_map<std::string, VariableInfo>> variableScopes;
    std::unordered_map<std::string, FunctionInfo> functions;

    void pushScope();
    void popScope();

    void declareVariable(const std::string &name, Type type = Type::UNKNOWN);
    bool isVariableDeclared(const std::string &name) const;
    Type getVariableType(const std::string &name) const;

    void registerFunction(const FuncDeclNode *func);
    bool isFunctionDeclared(const std::string &name) const;
    FunctionInfo getFunctionInfo(const std::string &name) const;

    Type analyzeNode(NodePtr &node);
    Type analyzeAssign(AssignNode *n);
    Type analyzeFuncDecl(FuncDeclNode *n);
    Type analyzeBinary(BinaryOpNode *n);
    Type analyzeVar(VarNode *n);
    Type analyzeFuncCall(FuncCallNode *n);
    Type analyzeNumber(const NumberNode *n);

    Type checkBinaryOpTypes(const std::string &op, Type left, Type right);

public:
    SemanticAnalyzer();
    void analyze(std::vector<NodePtr> &ast);
};

#endif