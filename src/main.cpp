#include <iostream>
#include <sstream>
#include <string>

#include "../include/lexer.h"
#include "../include/parser.h"
#include "../include/semantic.h"
#include "../include/codegen.h"
#include "../include/interpreter.h"

int main() {
    std::cout << "Digite o código da linguagem (uma linha por vez, termine com linha vazia):\n";

    // Lê múltiplas linhas do usuário
    std::stringstream buffer;
    std::string linha;

    while (true) {
        std::getline(std::cin, linha);
        if (linha.empty()) break; // linha vazia termina a entrada
        buffer << linha << '\n';
    }

    std::string codigo = buffer.str();

    // Mostra o código recebido
    std::cout << "\nCódigo recebido:\n" << codigo << "\n";

    // Lexer
    Lexer lexer(codigo);
    auto tokens = lexer.tokenize();

    std::cout << "\nTokens:\n";
    for (auto &t : tokens) {
        std::cout << "line:" << t.line << " col:" << t.column << " "
                  << tokenTypeToString(t.type)
                  << "('" << t.value << "')\n";
    }

    // Parser
    Parser parser(tokens);
    std::vector<NodePtr> astList;

    try {
        astList = parser.parseAll();

        std::cout << "\nAST:\n";
        for (auto &n : astList) {
            n->prettyPrint();
        }

    } catch (const std::exception &e) {
        std::cerr << "Erro de parser: " << e.what() << "\n";
        return 1;
    }

    // Semântica
    try {
        SemanticAnalyzer sem;
        sem.analyze(astList);
        std::cout << "\nAnálise semântica OK!\n";

    } catch (const std::exception &e) {
        std::cerr << e.what() << "\n";
        return 1;
    }

    // Geração de código/execução de código
    try {
    CodeGenerator codegen;
    codegen.generateCode(astList);
    codegen.printCode();

    Interpreter interpreter(codegen.getCodeLines());
    interpreter.execute();

    } catch (const std::exception &e) {
        std::cerr << "Erro na geração/execução de código:: " << e.what() << "\n";
        return 1;
    }

    return 0;
}