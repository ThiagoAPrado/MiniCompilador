#ifndef TOKEN_H
#define TOKEN_H

#include <string>

enum class TokenType {
    ID, NUM, FUNC,
    OP_ARIT, ATRIB,
    LPAREN, RPAREN, COMMA,
    END_OF_FILE, INVALID
};

struct Token {
    TokenType type;
    std::string value;
    int line;
    int column;
};

inline std::string tokenTypeToString(TokenType t) {
    switch (t) {
        case TokenType::ID: return "ID";
        case TokenType::NUM: return "NUM";
        case TokenType::FUNC: return "FUNC";
        case TokenType::OP_ARIT: return "OP_ARIT";
        case TokenType::ATRIB: return "ATRIB";
        case TokenType::LPAREN: return "LPAREN";
        case TokenType::RPAREN: return "RPAREN";
        case TokenType::COMMA: return "COMMA";
        case TokenType::END_OF_FILE: return "EOF";
        default: return "INVALID";
    }
}

#endif
