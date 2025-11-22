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

void SemanticAnalyzer::declareVariable(const std::string &name, Type type) {
    variableScopes.back()[name] = VariableInfo{type, true};
}

bool SemanticAnalyzer::isVariableDeclared(const std::string &name) const {
    for (auto it = variableScopes.rbegin(); it != variableScopes.rend(); ++it) {
        if (it->count(name)) return true;
    }
    return false;
}

Type SemanticAnalyzer::getVariableType(const std::string &name) const {
    for (auto it = variableScopes.rbegin(); it != variableScopes.rend(); ++it) {
        auto found = it->find(name);
        if (found != it->end()) return found->second.type;
    }
    return Type::UNKNOWN;
}

void SemanticAnalyzer::registerFunction(const FuncDeclNode *func) {
    if (functions.count(func->name)) {
        throw SemanticError("função '" + func->name + "' já declarada.");
    }

    // Determina tipo de retorno da função analisando o corpo
    // Como não podemos modificar, vamos apenas verificar
    Type returnType = Type::UNKNOWN;
    if (auto num = dynamic_cast<const NumberNode*>(func->body.get())) {
        returnType = num->type;
    } else if (auto var = dynamic_cast<const VarNode*>(func->body.get())) {
        returnType = getVariableType(var->name);
    }

    functions[func->name] = FunctionInfo{
        func,
        static_cast<int>(func->params.size()),
        returnType
    };
}

bool SemanticAnalyzer::isFunctionDeclared(const std::string &name) const {
    return functions.count(name) > 0;
}

FunctionInfo SemanticAnalyzer::getFunctionInfo(const std::string &name) const {
    auto it = functions.find(name);
    if (it == functions.end()) {
        throw SemanticError("função '" + name + "' não encontrada.");
    }
    return it->second;
}

Type SemanticAnalyzer::analyzeNode(NodePtr &node) {
    if (!node) return Type::UNKNOWN;

    if (auto n = dynamic_cast<AssignNode*>(node.get())) {
        return analyzeAssign(n);
    } else if (auto n = dynamic_cast<FuncDeclNode*>(node.get())) {
        return analyzeFuncDecl(n);
    } else if (auto n = dynamic_cast<BinaryOpNode*>(node.get())) {
        return analyzeBinary(n);
    } else if (auto n = dynamic_cast<VarNode*>(node.get())) {
        return analyzeVar(n);
    } else if (auto n = dynamic_cast<FuncCallNode*>(node.get())) {
        return analyzeFuncCall(n);
    } else if (auto n = dynamic_cast<const NumberNode*>(node.get())) {
        return analyzeNumber(n);
    }

    return Type::UNKNOWN;
}

Type SemanticAnalyzer::analyzeAssign(AssignNode *n) {
    Type exprType = analyzeNode(n->expr);
    declareVariable(n->name, exprType);
    // Não modificamos n->type pois é const
    return exprType;
}

Type SemanticAnalyzer::analyzeFuncDecl(FuncDeclNode *n) {
    pushScope();

    std::unordered_map<std::string, bool> seen;
    for (const std::string &p : n->params) {
        if (seen.count(p)) {
            throw SemanticError("parâmetro duplicado '" + p + "' na função '" + n->name + "'");
        }
        seen[p] = true;
        declareVariable(p, Type::UNKNOWN);
    }

    Type returnType = analyzeNode(n->body);
    // Não modificamos n->type pois é const

    popScope();
    return returnType;
}

Type SemanticAnalyzer::analyzeBinary(BinaryOpNode *n) {
    Type leftType = analyzeNode(n->left);
    Type rightType = analyzeNode(n->right);
    
    Type resultType = checkBinaryOpTypes(n->op, leftType, rightType);
    // Não modificamos n->type pois é const
    return resultType;
}

Type SemanticAnalyzer::analyzeVar(VarNode *n) {
    if (!isVariableDeclared(n->name)) {
        throw SemanticError("variável '" + n->name + "' não declarada.");
    }
    
    Type varType = getVariableType(n->name);
    // Não modificamos n->type pois é const
    return varType;
}

Type SemanticAnalyzer::analyzeFuncCall(FuncCallNode *n) {
    if (!isFunctionDeclared(n->name)) {
        throw SemanticError("função '" + n->name + "' não declarada.");
    }

    FunctionInfo funcInfo = getFunctionInfo(n->name);
    int expected = funcInfo.paramCount;
    int received = static_cast<int>(n->args.size());

    if (expected != received) {
        throw SemanticError("função '" + n->name + "' esperava " +
                           std::to_string(expected) + " argumentos, recebeu " +
                           std::to_string(received) + ".");
    }

    // Analisa tipos dos argumentos
    for (auto &arg : n->args) {
        analyzeNode(arg);
    }

    // Não modificamos n->type pois é const
    return funcInfo.returnType;
}

Type SemanticAnalyzer::analyzeNumber(const NumberNode *n) {
    // O tipo já é determinado na construção do NumberNode
    return n->type;
}

Type SemanticAnalyzer::checkBinaryOpTypes(const std::string &op, Type left, Type right) {
    // Regras de tipo para operadores
    if (left == Type::UNKNOWN || right == Type::UNKNOWN) {
        return Type::UNKNOWN;
    }
    
    // Para operadores aritméticos, promove para float se algum operando for float
    if (op == "+" || op == "-" || op == "*" || op == "/" || op == "^") {
        if (left == Type::FLOAT || right == Type::FLOAT) {
            return Type::FLOAT;
        }
        return Type::INT;
    }
    
    return Type::UNKNOWN;
}

void SemanticAnalyzer::analyze(std::vector<NodePtr> &ast) {
    // Primeira passada: registrar funções
    for (const auto &node : ast) {
        if (auto f = dynamic_cast<const FuncDeclNode*>(node.get())) {
            registerFunction(f);
        }
    }

    // Segunda passada: análise completa
    for (auto &node : ast) {
        analyzeNode(node);
    }
}