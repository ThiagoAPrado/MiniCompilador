#ifndef CODEGEN_H
#define CODEGEN_H

#include "ast.h"
#include <vector>
#include <string>
#include <iostream>

class CodeGenerator {
private:
    std::vector<std::string> codeLines;
    int tempCounter;
    
    std::string newTemp();
    std::string processNode(const Node* node);
    void processFunctionDeclaration(FuncDeclNode* funcDecl);
    
public:
    CodeGenerator();
    void generateCode(const std::vector<NodePtr> &ast);
    void printCode() const;
    const std::vector<std::string>& getCodeLines() const { return codeLines; }
};

#endif