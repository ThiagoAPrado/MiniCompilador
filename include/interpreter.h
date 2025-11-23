#ifndef INTERPRETER_H
#define INTERPRETER_H

#include <vector>
#include <string>
#include <unordered_map>
#include <iostream>
#include <utility>

// Estrutura da Função traduzida do IR
struct FunctionIR {
    std::vector<std::string> params;   // parâmetros formais
    std::vector<std::string> body;     // linhas internas do IR
};

class Interpreter {
private:
    // Variáveis globais
    std::unordered_map<std::string, double> variables;

    // Funções encontradas no IR
    std::unordered_map<std::string, FunctionIR> functions;

    // Código principal (fora das funções)
    std::vector<std::string> mainLines;

    // Pilha de escopos (um map para cada chamada)
    std::vector<std::unordered_map<std::string, double>> callStack;

    // ==== Métodos internos ====
    double getValue(const std::string& name);
    void setValue(const std::string& name, double value);

    void executeInstruction(const std::string& line);

    // Executa múltiplas instruções e busca `return`
    std::pair<bool, double> executeLines(const std::vector<std::string>& lines);

    // Chama uma função do IR
    double callFunction(const std::string& name, const std::vector<double>& args);

public:
    Interpreter(const std::vector<std::string>& lines);
    void execute();
    void printVariables() const;
};

#endif
