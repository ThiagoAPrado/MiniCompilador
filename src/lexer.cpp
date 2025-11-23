#include "../include/lexer.h"
#include <cctype>

Lexer::Lexer(const std::string &src) : source(src), pos(0), line(1), column(1) {}

char Lexer::peek() const {
    return pos < source.size() ? source[pos] : '\0';
}

char Lexer::get() {
    char c = peek();
    if (c == '\n') { line++; column = 1; }
    else column++;
    pos++;
    return c;
}

void Lexer::skipWhitespace() {
    while (isspace((unsigned char)peek())) get();
}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;

    while (pos < source.size()) {
        skipWhitespace();
        char c = peek();
        if (c == '\0') break;

        int tokLine = line;
        int tokCol = column;

        if (isalpha((unsigned char)c)) {
            std::string id;
            while (isalnum((unsigned char)peek())) id += get();
            TokenType type = (id == "funcao") ? TokenType::FUNC : TokenType::ID;
            tokens.push_back({type, id, tokLine, tokCol});
        }
        else if (isdigit((unsigned char)c)) {
            std::string num;
            bool dotSeen = false;
            while (isdigit((unsigned char)peek()) || (!dotSeen && peek() == '.')) {
                if (peek() == '.') dotSeen = true;
                num += get();
            }
            tokens.push_back({TokenType::NUM, num, tokLine, tokCol});
        }
        else {
            switch (c) {
                case '+': case '-': case '*': case '/': case '^': {
                    std::string s(1, get());
                    tokens.push_back({TokenType::OP_ARIT, s, tokLine, tokCol});
                    break;
                }
                case '=':
                    get();
                    tokens.push_back({TokenType::ATRIB, "=", tokLine, tokCol});
                    break;
                case '(':
                    get();
                    tokens.push_back({TokenType::LPAREN, "(", tokLine, tokCol});
                    break;
                case ')':
                    get();
                    tokens.push_back({TokenType::RPAREN, ")", tokLine, tokCol});
                    break;
                case ',':
                    get();
                    tokens.push_back({TokenType::COMMA, ",", tokLine, tokCol});
                    break;
                default:
                    {
                        std::string s(1, get());
                        tokens.push_back({TokenType::INVALID, s, tokLine, tokCol});
                    }
                    break;
            }
        }
    }

    tokens.push_back({TokenType::END_OF_FILE, "", line, column});
    return tokens;
}
