#ifndef PARSER_H
#define PARSER_H

#include "token.h"
#include "ast.h"
#include <vector>
#include <memory>

class Parser {
private:
    const std::vector<Token> tokens;
    size_t idx;

    const Token& current() const;
    void advance();
    bool accept(TokenType t);
    bool acceptValue(TokenType t, const std::string &val);
    void expect(TokenType t, const std::string &msg);

    NodePtr parseProgram();       
    NodePtr parseDeclaration();   
    NodePtr parseAssignment();    
    NodePtr parseExpression();    
    NodePtr parseTerm();          
    NodePtr parsePower();         
    NodePtr parseFactor();        
    std::vector<std::string> parseParameters();
    std::vector<NodePtr> parseArguments();

public:
    Parser(const std::vector<Token>& toks);
    NodePtr parse(); 
    std::vector<NodePtr> parseAll(); 
};

#endif
