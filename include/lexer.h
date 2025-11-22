#ifndef LEXER_H
#define LEXER_H

#include "token.h"
#include <string>
#include <vector>

class Lexer {
private:
    std::string source;
    size_t pos;
    int line, column;

    char peek() const;
    char get();
    void skipWhitespace();

public:
    Lexer(const std::string &src);
    std::vector<Token> tokenize();
};

#endif
