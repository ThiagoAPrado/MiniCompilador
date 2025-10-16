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

    // grammar functions
    NodePtr parseProgram();         // parse a single top-level statement (declaration or assignment)
    NodePtr parseDeclaration();     // 'funcao' ID '(' params ')' '=' expr
    NodePtr parseAssignment();      // ID '=' expr
    NodePtr parseExpression();      // parse + -
    NodePtr parseTerm();            // parse * /
    NodePtr parsePower();           // parse ^ (right-assoc)
    NodePtr parseFactor();          // NUM | ID | '(' expr ')' | ID '(' args ')'
    std::vector<std::string> parseParameters();
    std::vector<NodePtr> parseArguments();

public:
    Parser(const std::vector<Token>& toks);

    // Parse uma única instrução (como antes)
    NodePtr parse(); 

    // Parse todas as instruções até EOF
    std::vector<NodePtr> parseAll(); 
};

#endif
