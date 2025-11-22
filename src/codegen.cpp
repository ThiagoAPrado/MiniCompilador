#include "../include/codegen.h"
#include <iostream>

CodeGenerator::CodeGenerator() : tempCounter(0) {}

std::string CodeGenerator::newTemp() {
    return "t" + std::to_string(tempCounter++);
}

void CodeGenerator::generateCode(const std::vector<NodePtr> &ast) {
    codeLines.clear();
    tempCounter = 0;
    
    // REMOVA ESTA LINHA: codeLines.push_back("; === CÓDIGO DE TRÊS ENDEREÇOS ===");
    // E DEIXE APENAS:
    codeLines.push_back("=== CÓDIGO INTERMEDIÁRIO (TRÊS ENDEREÇOS) ===");
    
    // O resto do código permanece igual...
    for (const auto &node : ast) {
        if (auto funcDecl = dynamic_cast<FuncDeclNode*>(node.get())) {
            processFunctionDeclaration(funcDecl);
        }
    }
    
    for (const auto &node : ast) {
        if (!node) continue;
        if (dynamic_cast<FuncDeclNode*>(node.get())) continue;
        
        if (auto assign = dynamic_cast<AssignNode*>(node.get())) {
            std::string value = processNode(assign->expr.get());
            if (!value.empty()) {
                codeLines.push_back(assign->name + " = " + value);
            }
        }
    }
    
    codeLines.push_back(""); // linha final em branco
}

// O resto do arquivo permanece igual...
void CodeGenerator::processFunctionDeclaration(FuncDeclNode* funcDecl) {
    codeLines.push_back("func_" + funcDecl->name + ":");
    
    for (const auto& param : funcDecl->params) {
        codeLines.push_back("  param " + param);
    }
    
    std::string bodyResult = processNode(funcDecl->body.get());
    if (!bodyResult.empty()) {
        codeLines.push_back("  return " + bodyResult);
    }
    
    codeLines.push_back("end_" + funcDecl->name + ":");
    codeLines.push_back(""); // linha em branco
}

std::string CodeGenerator::processNode(const Node* node) {
    if (!node) return "";
    
    if (auto num = dynamic_cast<const NumberNode*>(node)) {
        return num->value;
    }
    else if (auto var = dynamic_cast<const VarNode*>(node)) {
        return var->name;
    }
    else if (auto binary = dynamic_cast<const BinaryOpNode*>(node)) {
        std::string left = processNode(binary->left.get());
        std::string right = processNode(binary->right.get());
        
        if (!left.empty() && !right.empty()) {
            std::string temp = newTemp();
            codeLines.push_back("  " + temp + " = " + left + " " + binary->op + " " + right);
            return temp;
        }
    }
    else if (auto funcCall = dynamic_cast<const FuncCallNode*>(node)) {
        std::string argsStr;
        for (size_t i = 0; i < funcCall->args.size(); ++i) {
            std::string arg = processNode(funcCall->args[i].get());
            if (!arg.empty()) {
                codeLines.push_back("  arg" + std::to_string(i) + " = " + arg);
                if (i > 0) argsStr += ", ";
                argsStr += arg;
            }
        }
        
        std::string temp = newTemp();
        codeLines.push_back("  " + temp + " = call " + funcCall->name + " " + std::to_string(funcCall->args.size()));
        return temp;
    }
    
    return "";
}

void CodeGenerator::printCode() const {
    std::cout << "\n"; // REMOVA o título duplicado aqui também
    for (const auto &line : codeLines) {
        std::cout << line << std::endl;
    }
}