// filename: interpreter.cpp
#include "../include/interpreter.h"
#include <sstream>
#include <cmath>
#include <algorithm>
#include <unordered_map>
#include <vector>
#include <string>
#include <iostream>
#include <cctype>

Interpreter::Interpreter(const std::vector<std::string>& lines) {
    // Parse inicial: separar definições de função e linhas principais
    bool inFunc = false;
    std::string currentFuncName;
    std::vector<std::string> currentFuncBody;

    for (size_t i = 0; i < lines.size(); ++i) {
        std::string raw = lines[i];
        // trim left
        size_t start = raw.find_first_not_of(" \t\r\n");
        std::string line = (start == std::string::npos) ? std::string() : raw.substr(start);

        if (line.empty()) continue;

        // detect function label
        if (!inFunc && line.rfind("func_", 0) == 0 && line.back() == ':') {
            inFunc = true;
            currentFuncName = line.substr(5, line.size() - 6); // remove "func_" prefix and ":" suffix
            currentFuncBody.clear();
            continue;
        }

        // detect end of function
        if (inFunc && line.rfind("end_", 0) == 0 && line.back() == ':') {
            // store function
            // extract params from lines that start with "param "
            FunctionIR fir;
            for (const auto &l : currentFuncBody) {
                size_t s = l.find_first_not_of(" \t");
                std::string tl = (s == std::string::npos) ? std::string() : l.substr(s);
                if (tl.rfind("param ", 0) == 0) {
                    std::istringstream iss(tl);
                    std::string kw, pname;
                    iss >> kw >> pname;
                    if (!pname.empty()) fir.params.push_back(pname);
                }
            }
            fir.body = currentFuncBody;
            functions[currentFuncName] = std::move(fir);

            inFunc = false;
            currentFuncName.clear();
            currentFuncBody.clear();
            continue;
        }

        if (inFunc) {
            currentFuncBody.push_back(line);
        } else {
            // linha fora de função -> executável na main
            mainLines.push_back(line);
        }
    }

    // inicializa call stack com escopo global
    callStack.clear();
    callStack.emplace_back(); // escopo global
}

//
// Helpers para trim e verificação
//
static inline std::string trim(const std::string &s) {
    size_t a = s.find_first_not_of(" \t\r\n");
    if (a == std::string::npos) return "";
    size_t b = s.find_last_not_of(" \t\r\n");
    return s.substr(a, b - a + 1);
}

static inline bool startsWith(const std::string &s, const std::string &prefix) {
    return s.size() >= prefix.size() && s.compare(0, prefix.size(), prefix) == 0;
}

//
// Pesquisa valor de nome nas scopes (do topo para baixo)
//
double Interpreter::getValue(const std::string& name) {
    std::string n = name;
    if (n.empty()) return 0.0;

    // detect number literal (inteiro, negativo, float)
    bool isNumber = false;
    size_t idx = 0;
    if (n[0] == '-' || n[0] == '+') idx = 1;
    bool dotFound = false;
    for (size_t i = idx; i < n.size(); ++i) {
        if (n[i] == '.') {
            if (dotFound) { isNumber = false; break; }
            dotFound = true;
            continue;
        }
        if (!std::isdigit((unsigned char)n[i])) {
            isNumber = false;
            break;
        }
        isNumber = true;
    }
    if (isNumber) {
        try {
            return std::stod(n);
        } catch (...) {
            return 0.0;
        }
    }

    // busca nas scopes (do topo para baixo)
    for (auto it = callStack.rbegin(); it != callStack.rend(); ++it) {
        auto found = it->find(n);
        if (found != it->end()) return found->second;
    }

    // se não encontrada, retorna 0.0 por padrão
    return 0.0;
}

void Interpreter::setValue(const std::string& name, double value) {
    if (callStack.empty()) {
        callStack.emplace_back();
    }
    // define no escopo atual (topo da pilha)
    callStack.back()[name] = value;
}

//
// Executa uma lista de linhas (usada para corpo de função)
// Retorna pair<retornou, valor> onde 'retornou' indica se houve return.
//
std::pair<bool,double> Interpreter::executeLines(const std::vector<std::string>& lines) {
    for (const auto &raw : lines) {
        std::string line = trim(raw);
        if (line.empty()) continue;

        // Ignora labels e "param" durante execução do corpo
        if (startsWith(line, "func_") || startsWith(line, "end_") || startsWith(line, "param ")) {
            continue;
        }

        // Se for "return <expr>"
        if (startsWith(line, "return ")) {
            std::string expr = trim(line.substr(7));
            double val = getValue(expr);
            return {true, val};
        }

        // Executa instrução normal
        // Ex.: "t0 = a + b" OR "x = t1" OR "arg0 = 10" OR "t4 = call calc 3"
        std::istringstream iss(line);
        std::string dest;
        if (!(iss >> dest)) continue;

        // possivelmente dest termina com ':' (label) -> ignorar
        if (!dest.empty() && dest.back() == ':') continue;

        std::string eq;
        if (!(iss >> eq)) continue;

        if (eq != "=") {
            // Caso estranho: se não houver '=' consideramos que é instrução inválida e pulamos
            continue;
        }

        std::string token1;
        if (!(iss >> token1)) continue;

        // Se não há mais tokens -> atribuição simples: dest = token1
        std::string token2;
        if (!(iss >> token2)) {
            double v = getValue(token1);
            setValue(dest, v);
            std::cout << "  " << dest << " = " << v << std::endl;
            continue;
        }

        // Agora token1 e token2 existem. Podemos ter formas:
        // 1) dest = call funcName numArgs    -> token1 == "call", token2 = funcName, next = numArgs
        // 2) dest = token1 op token2         -> token1 is left, token2 is op, next is right
        // 3) dest = token1 (handled above)
        std::string token3;
        iss >> token3; // may be empty

        if (token1 == "call") {
            // forma: dest = call funcName numArgs
            std::string funcName = token2;
            int numArgs = 0;
            if (!token3.empty()) {
                try { numArgs = std::stoi(token3); }
                catch (...) { numArgs = 0; }
            } else {
                if (!(iss >> numArgs)) numArgs = 0;
            }

            // recolhe argumentos a partir de arg0..argN-1 do escopo atual
            std::vector<double> argValues;
            for (int i = 0; i < numArgs; ++i) {
                std::string an = "arg" + std::to_string(i);
                double av = getValue(an);
                argValues.push_back(av);
            }

            double callResult = 0.0;
            if (!funcName.empty()) {
                callResult = callFunction(funcName, argValues);
            }

            setValue(dest, callResult);
            std::cout << "  " << dest << " = " << callResult << " (call " << funcName << ")\n";
            continue;
        } else if (token2 == "call") {
            // forma alternativa: dest = left call funcName  (unlikely) -> treat as call with left ignored
            std::string funcName = token3;
            int numArgs = 0;
            if (!(iss >> numArgs)) numArgs = 0;

            std::vector<double> argValues;
            for (int i = 0; i < numArgs; ++i) {
                std::string an = "arg" + std::to_string(i);
                double av = getValue(an);
                argValues.push_back(av);
            }

            double callResult = callFunction(funcName, argValues);
            setValue(dest, callResult);
            std::cout << "  " << dest << " = " << callResult << " (call " << funcName << ")\n";
            continue;
        } else {
            // operador binário: token1 op token3 (token2 is op if token3 present)
            if (!token3.empty()) {
                double v1 = getValue(token1);
                double v2 = getValue(token3);
                double res = 0.0;
                if (token2 == "+") res = v1 + v2;
                else if (token2 == "-") res = v1 - v2;
                else if (token2 == "*") res = v1 * v2;
                else if (token2 == "/") {
                    if (v2 == 0.0) res = std::nan("");
                    else res = v1 / v2;
                } else if (token2 == "^") res = std::pow(v1, v2);
                else {
                    // operador desconhecido -> fallback para valor de token1
                    res = getValue(token1);
                }
                setValue(dest, res);
                std::cout << "  " << dest << " = " << v1 << " " << token2 << " " << v2 << " = " << res << std::endl;
                continue;
            } else {
                // não há token3 -> forma inesperada; atribui token1
                double v1 = getValue(token1);
                setValue(dest, v1);
                std::cout << "  " << dest << " = " << v1 << std::endl;
                continue;
            }
        }
    }

    return {false, 0.0};
}

double Interpreter::callFunction(const std::string &name, const std::vector<double> &args) {
    auto it = functions.find(name);
    if (it == functions.end()) {
        std::cerr << "Erro: função '" << name << "' não encontrada.\n";
        return 0.0;
    }

    const FunctionIR &fir = it->second;

    // Verifica correspondência de número de parâmetros (se disponíveis)
    if (!fir.params.empty() && static_cast<int>(fir.params.size()) != static_cast<int>(args.size())) {
        std::cerr << "Aviso: função '" << name << "' esperava " << fir.params.size()
                  << " args, recebeu " << args.size() << ".\n";
    }

    // cria novo escopo (stack frame)
    std::unordered_map<std::string, double> newScope;

    // mapeia parâmetros (até o menor tamanho)
    int limit = std::min(static_cast<int>(fir.params.size()), static_cast<int>(args.size()));
    for (int i = 0; i < limit; ++i) {
        newScope[fir.params[i]] = args[i];
    }

    // empilha o novo escopo
    callStack.push_back(newScope);

    // executa o corpo da função até encontrar return
    auto retPair = executeLines(fir.body);

    // desempilha
    callStack.pop_back();

    if (retPair.first) {
        return retPair.second;
    } else {
        // se não retornou explicitamente, retorna 0
        return 0.0;
    }
}

void Interpreter::executeInstruction(const std::string& line) {
    // esta função não é usada diretamente agora; mantemos para compatibilidade
    // com sua interface original — usamos execute() que itera sobre mainLines.
    std::string l = trim(line);
    if (l.empty()) return;
    // se for atribuição simples, delega para executeLines com um vetor de 1 linha
    std::vector<std::string> tmp{l};
    executeLines(tmp);
}

void Interpreter::execute() {
    std::cout << "\n=== EXECUÇÃO DO CÓDIGO ===\n";

    // Antes de executar, pode haver instruções 'argN = value' intercaladas.
    // Executamos cada linha principal; quando encontrar um call, ele fará a chamada
    for (const auto &line : mainLines) {
        if (line.empty() || line.find("===") != std::string::npos) continue;
        std::cout << "Executando: " << line << std::endl;

        // Trata labels/params/retornos na main (não devem aparecer normalmente)
        if (startsWith(line, "func_") || startsWith(line, "end_") || startsWith(line, "param ")) {
            continue;
        }

        // Execução de linha normal: se for atribuição simples ou call, executeLines trata
        std::vector<std::string> single{line};
        executeLines(single);
    }

    printVariables();
}

void Interpreter::printVariables() const {
    std::cout << "\n=== VARIÁVEIS FINAIS ===\n";
    if (!callStack.empty()) {
        std::unordered_map<std::string, double> merged;
        for (const auto &scope : callStack) {
            for (const auto &kv : scope) merged[kv.first] = kv.second;
        }

        for (const auto &kv : merged) {
            if (!kv.first.empty() && kv.first[0] == 't') continue; // ignora temporários
            std::cout << kv.first << " = " << kv.second << std::endl;
        }
    } else {
        std::cout << "(nenhuma variável)\n";
    }
}
