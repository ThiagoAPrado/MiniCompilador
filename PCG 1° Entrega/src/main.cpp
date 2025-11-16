#include <iostream>
#include <sstream>
#include <string>
#include "../include/lexer.h"
#include "../include/parser.h"

int main() {
    std::cout << "Digite o código da linguagem (uma linha por vez, termine com linha vazia):\n";

    // Lê múltiplas linhas do usuário
    std::stringstream buffer;
    std::string linha;
    while (true) {
        std::getline(std::cin, linha);
        if (linha.empty()) break;   // linha vazia indica fim do input
        buffer << linha << '\n';
    }

    std::string codigo = buffer.str();

    // Mostra o código recebido
    std::cout << "\nCódigo recebido:\n" << codigo << "\n";

    // Lexer
    Lexer lexer(codigo);
    auto tokens = lexer.tokenize();

    // Mostra os tokens
    std::cout << "\nTokens:\n";
    for (auto &t : tokens) {
        std::cout << "line:" << t.line << " col:" << t.column << " "
         << tokenTypeToString(t.type) << "('" << t.value << "')" << "\n";
    }

    // Parser
    try {
        Parser parser(tokens);
        auto ast_list = parser.parseAll();  // processa todas as instruções

        std::cout << "\nAST:\n";
        for (auto &ast : ast_list) {
            ast->prettyPrint();
        }
    } catch (std::exception &e) {
        std::cerr << "Erro de parser: " << e.what() << "\n";
    }

    return 0;
}
