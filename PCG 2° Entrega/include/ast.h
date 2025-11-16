#ifndef AST_H
#define AST_H

#include <memory>
#include <string>
#include <vector>
#include <iostream>

struct Node {
    virtual ~Node() = default;
    virtual void prettyPrint(int indent=0) const = 0;
};

using NodePtr = std::unique_ptr<Node>;

inline void printIndent(int n) {
    for (int i=0;i<n;i++) std::cout << "  ";
}

struct NumberNode : Node {
    std::string value;
    NumberNode(const std::string &v): value(v) {}
    void prettyPrint(int indent=0) const override {
        printIndent(indent);
        std::cout << "Number(" << value << ")\n";
    }
};

struct VarNode : Node {
    std::string name;
    VarNode(const std::string &n): name(n) {}
    void prettyPrint(int indent=0) const override {
        printIndent(indent);
        std::cout << "Var(" << name << ")\n";
    }
};

struct BinaryOpNode : Node {
    std::string op;
    NodePtr left, right;
    BinaryOpNode(std::string o, NodePtr l, NodePtr r): op(std::move(o)), left(std::move(l)), right(std::move(r)) {}
    void prettyPrint(int indent=0) const override {
        printIndent(indent);
        std::cout << "BinaryOp(" << op << ")\n";
        if (left) left->prettyPrint(indent+1);
        if (right) right->prettyPrint(indent+1);
    }
};

struct FuncCallNode : Node {
    std::string name;
    std::vector<NodePtr> args;
    FuncCallNode(std::string n, std::vector<NodePtr> a): name(std::move(n)), args(std::move(a)) {}
    void prettyPrint(int indent=0) const override {
        printIndent(indent);
        std::cout << "FuncCall(" << name << ")\n";
        for (const auto &a : args) a->prettyPrint(indent+1);
    }
};

struct AssignNode : Node {
    std::string name;
    NodePtr expr;
    AssignNode(std::string n, NodePtr e): name(std::move(n)), expr(std::move(e)) {}
    void prettyPrint(int indent=0) const override {
        printIndent(indent);
        std::cout << "Assign(" << name << ")\n";
        if (expr) expr->prettyPrint(indent+1);
    }
};

struct FuncDeclNode : Node {
    std::string name;
    std::vector<std::string> params;
    NodePtr body;
    FuncDeclNode(std::string n, std::vector<std::string> p, NodePtr b)
        : name(std::move(n)), params(std::move(p)), body(std::move(b)) {}
    void prettyPrint(int indent=0) const override {
        printIndent(indent);
        std::cout << "FuncDecl(" << name << ")\n";
        printIndent(indent+1);
        std::cout << "Params:\n";
        for (auto &p: params) { printIndent(indent+2); std::cout << p << "\n"; }
        printIndent(indent+1);
        std::cout << "Body:\n";
        if (body) body->prettyPrint(indent+2);
    }
};

#endif
