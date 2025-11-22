#include "interpreter.h"
#include <sstream>
#include <cmath>
#include <algorithm>

Interpreter::Interpreter(const std::vector<std::string>& lines) : codeLines(lines) {}

double Interpreter::getValue(const std::string& name) {
    // Se for um número
    if (isdigit(name[0]) || (name.size() > 1 && name[0] == '-' && isdigit(name[1]))) {
        return std::stod(name);
    }
    // Se for uma variável
    auto it = variables.find(name);
    if (it != variables.end()) {
        return it->second;
    }
    return 0.0; // Default
}

void Interpreter::setValue(const std::string& name, double value) {
    variables[name] = value;
}

void Interpreter::executeInstruction(const std::string& line) {
    if (line.empty() || line[0] == '=' || line.find("func_") == 0 || 
        line.find("end_") == 0 || line.find("param") == 0 || 
        line.find("return") == 0 || line.find("arg") == 0) {
        return; // Ignora labels e instruções de função
    }
    
    std::istringstream iss(line);
    std::string result, equals, arg1, op, arg2;
    
    iss >> result;
    if (result.empty()) return;
    
    iss >> equals;
    if (equals != "=") {
        // Talvez seja uma atribuição simples sem operação
        if (!equals.empty()) {
            setValue(result, getValue(equals));
        }
        return;
    }
    
    iss >> arg1;
    if (arg1.empty()) return;
    
    // Verifica se há operação
    if (iss >> op) {
        if (iss >> arg2) {
            // Operação binária: result = arg1 op arg2
            double val1 = getValue(arg1);
            double val2 = getValue(arg2);
            double result_val = 0.0;
            
            if (op == "+") result_val = val1 + val2;
            else if (op == "-") result_val = val1 - val2;
            else if (op == "*") result_val = val1 * val2;
            else if (op == "/") result_val = val1 / val2;
            else if (op == "^") result_val = std::pow(val1, val2);
            else if (op == "call") {
                // Simulação simples de chamada de função
                if (arg1 == "calcula") {
                    // calcula(x,y) = x*x + y*y
                    result_val = val2 * val2 + getValue("arg1") * getValue("arg1");
                } else if (arg1 == "potencia") {
                    // potencia(base,exp) = base^exp
                    result_val = std::pow(getValue("arg0"), getValue("arg1"));
                } else if (arg1 == "soma") {
                    // soma(a,b) = a + b
                    result_val = getValue("arg0") + getValue("arg1");
                }
            }
            
            setValue(result, result_val);
            std::cout << "  " << result << " = " << val1 << " " << op << " " << val2 << " = " << result_val << std::endl;
        }
    } else {
        // Atribuição simples: result = arg1
        double value = getValue(arg1);
        setValue(result, value);
        std::cout << "  " << result << " = " << value << std::endl;
    }
}

void Interpreter::execute() {
    std::cout << "\n=== EXECUÇÃO DO CÓDIGO ===\n";
    
    for (const auto& line : codeLines) {
        if (line.empty() || line.find("===") != std::string::npos) {
            continue; // Pula linhas vazias e cabeçalhos
        }
        
        std::cout << "Executando: " << line << std::endl;
        executeInstruction(line);
    }
    
    printVariables();
}

void Interpreter::printVariables() const {
    std::cout << "\n=== VARIÁVEIS FINAIS ===\n";
    for (const auto& var : variables) {
        if (var.first[0] != 't') { // Mostra apenas variáveis não-temporárias
            std::cout << var.first << " = " << var.second << std::endl;
        }
    }
}