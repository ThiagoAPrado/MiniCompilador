#include "../include/parser.h"
#include <stdexcept>
#include <iostream>

Parser::Parser(const std::vector<Token>& toks) : tokens(toks), idx(0) {}

std::vector<NodePtr> Parser::parseAll() {
    std::vector<NodePtr> statements;

    while (current().type != TokenType::END_OF_FILE) {
        statements.push_back(parseProgram());
    }

    return statements;
}

const Token& Parser::current() const {
    return tokens[idx];
}

void Parser::advance() {
    if (tokens[idx].type != TokenType::END_OF_FILE) idx++;
}

bool Parser::accept(TokenType t) {
    if (current().type == t) { advance(); return true; }
    return false;
}

bool Parser::acceptValue(TokenType t, const std::string &val) {
    if (current().type == t && current().value == val) { advance(); return true; }
    return false;
}

void Parser::expect(TokenType t, const std::string &msg) {
    if (current().type != t) {
        throw std::runtime_error("Parse error at line " + std::to_string(current().line) +
                                 " col " + std::to_string(current().column) +
                                 ": expected " + tokenTypeToString(t) + " (" + msg + "), got " +
                                 tokenTypeToString(current().type) + " '" + current().value + "'");
    }
    advance();
}

NodePtr Parser::parse() {
    if (current().type == TokenType::END_OF_FILE) return nullptr;
    NodePtr n = parseProgram();
    if (current().type != TokenType::END_OF_FILE) {
        throw std::runtime_error("Extra tokens after end of statement at line " +
                                 std::to_string(current().line));
    }
    return n;
}

NodePtr Parser::parseProgram() {
    if (current().type == TokenType::FUNC) {
        return parseDeclaration();
    }
    else if (current().type == TokenType::ID) {
        
        if (tokens.size() > idx+1 && tokens[idx+1].type == TokenType::ATRIB) {
            return parseAssignment();
        } else {
            
            NodePtr expr = parseExpression();
            return expr;
        }
    } else {
        throw std::runtime_error("Unexpected token at start of statement: " + current().value);
    }
}

NodePtr Parser::parseDeclaration() {
    
    expect(TokenType::FUNC, "'funcao' keyword");
    if (current().type != TokenType::ID) throw std::runtime_error("Expected function name after 'funcao'");
    std::string name = current().value;
    advance();
    expect(TokenType::LPAREN, "'(' after function name");
    std::vector<std::string> params = parseParameters();
    expect(TokenType::RPAREN, "')' after params");
    expect(TokenType::ATRIB, "'=' before function body");
    NodePtr body = parseExpression();
    return std::make_unique<FuncDeclNode>(name, params, std::move(body));
}

std::vector<std::string> Parser::parseParameters() {
    std::vector<std::string> out;
    if (current().type == TokenType::RPAREN) return out; 
    if (current().type != TokenType::ID) throw std::runtime_error("Expected parameter name");
    out.push_back(current().value);
    advance();
    while (accept(TokenType::COMMA)) {
        if (current().type != TokenType::ID) throw std::runtime_error("Expected parameter name after ','");
        out.push_back(current().value);
        advance();
    }
    return out;
}

NodePtr Parser::parseAssignment() {
    if (current().type != TokenType::ID) throw std::runtime_error("Expected identifier at assignment start");
    std::string name = current().value;
    advance();
    expect(TokenType::ATRIB, "'=' in assignment");
    NodePtr expr = parseExpression();
    return std::make_unique<AssignNode>(name, std::move(expr));
}

NodePtr Parser::parseExpression() {
    NodePtr node = parseTerm();
    while (current().type == TokenType::OP_ARIT && (current().value == "+" || current().value == "-")) {
        std::string op = current().value;
        advance();
        NodePtr right = parseTerm();
        node = std::make_unique<BinaryOpNode>(op, std::move(node), std::move(right));
    }
    return node;
}

NodePtr Parser::parseTerm() {
    NodePtr node = parsePower();
    while (current().type == TokenType::OP_ARIT && (current().value == "*" || current().value == "/")) {
        std::string op = current().value;
        advance();
        NodePtr right = parsePower();
        node = std::make_unique<BinaryOpNode>(op, std::move(node), std::move(right));
    }
    return node;
}

NodePtr Parser::parsePower() {
    NodePtr node = parseUnary();
    if (current().type == TokenType::OP_ARIT && current().value == "^") {
        std::string op = current().value;
        advance();
        NodePtr right = parsePower(); 
        node = std::make_unique<BinaryOpNode>(op, std::move(node), std::move(right));
    }
    return node;
}

NodePtr Parser::parseUnary() {
    if (current().type == TokenType::OP_ARIT && current().value == "-") {
        std::string op = current().value;
        advance();
        NodePtr operand = parseUnary();
        
        auto zeroNode = std::make_unique<NumberNode>("0");
        return std::make_unique<BinaryOpNode>(op, std::move(zeroNode), std::move(operand));
    }
    
    return parseFactor();
}

std::vector<NodePtr> Parser::parseArguments() {
    std::vector<NodePtr> out;
    if (current().type == TokenType::RPAREN) return out; 
    out.push_back(parseExpression());
    while (accept(TokenType::COMMA)) {
        out.push_back(parseExpression());
    }
    return out;
}

NodePtr Parser::parseFactor() {
    if (current().type == TokenType::NUM) {
        std::string v = current().value; 
        advance();
        return std::make_unique<NumberNode>(v);
    }
    else if (current().type == TokenType::ID) {
        std::string name = current().value;
        advance();
        if (accept(TokenType::LPAREN)) {
            
            std::vector<NodePtr> args = parseArguments();
            expect(TokenType::RPAREN, "')' after function arguments");
            return std::make_unique<FuncCallNode>(name, std::move(args));
        } else {
            return std::make_unique<VarNode>(name);
        }
    }
    else if (accept(TokenType::LPAREN)) {
        NodePtr inside = parseExpression();
        expect(TokenType::RPAREN, "')' expected");
        return inside;
    }
    else {
        throw std::runtime_error("Unexpected token in factor: " + current().value);
    }
}