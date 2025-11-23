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
    
    bool inFunc = false;
    std::string currentFuncName;
    std::vector<std::string> currentFuncBody;

    for (size_t i = 0; i < lines.size(); ++i) {
        std::string raw = lines[i];
        
        size_t start = raw.find_first_not_of(" \t\r\n");
        std::string line = (start == std::string::npos) ? std::string() : raw.substr(start);

        if (line.empty()) continue;
        
        if (!inFunc && line.rfind("func_", 0) == 0 && line.back() == ':') {
            inFunc = true;
            currentFuncName = line.substr(5, line.size() - 6); 
            currentFuncBody.clear();
            continue;
        }
        
        if (inFunc && line.rfind("end_", 0) == 0 && line.back() == ':') {
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
            
            mainLines.push_back(line);
        }
    }
    
    callStack.clear();
    callStack.emplace_back(); 
}

static inline std::string trim(const std::string &s) {
    size_t a = s.find_first_not_of(" \t\r\n");
    if (a == std::string::npos) return "";
    size_t b = s.find_last_not_of(" \t\r\n");
    return s.substr(a, b - a + 1);
}

static inline bool startsWith(const std::string &s, const std::string &prefix) {
    return s.size() >= prefix.size() && s.compare(0, prefix.size(), prefix) == 0;
}

double Interpreter::getValue(const std::string& name) {
    std::string n = name;
    if (n.empty()) return 0.0;

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
    
    for (auto it = callStack.rbegin(); it != callStack.rend(); ++it) {
        auto found = it->find(n);
        if (found != it->end()) return found->second;
    }

    return 0.0;
}

void Interpreter::setValue(const std::string& name, double value) {
    if (callStack.empty()) {
        callStack.emplace_back();
    }
    
    callStack.back()[name] = value;
}

std::pair<bool,double> Interpreter::executeLines(const std::vector<std::string>& lines) {
    for (const auto &raw : lines) {
        std::string line = trim(raw);
        if (line.empty()) continue;
        
        if (startsWith(line, "func_") || startsWith(line, "end_") || startsWith(line, "param ")) {
            continue;
        }
        
        if (startsWith(line, "return ")) {
            std::string expr = trim(line.substr(7));
            double val = getValue(expr);
            return {true, val};
        }

        std::istringstream iss(line);
        std::string dest;
        if (!(iss >> dest)) continue;
        
        if (!dest.empty() && dest.back() == ':') continue;

        std::string eq;
        if (!(iss >> eq)) continue;

        if (eq != "=") {
            continue;
        }

        std::string token1;
        if (!(iss >> token1)) continue;

        std::string token2;
        if (!(iss >> token2)) {
            double v = getValue(token1);
            setValue(dest, v);
            std::cout << "  " << dest << " = " << v << std::endl;
            continue;
        }

        std::string token3;
        iss >> token3; 

        if (token1 == "call") {
            
            std::string funcName = token2;
            int numArgs = 0;
            if (!token3.empty()) {
                try { numArgs = std::stoi(token3); }
                catch (...) { numArgs = 0; }
            } else {
                if (!(iss >> numArgs)) numArgs = 0;
            }

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
                    
                    res = getValue(token1);
                }
                setValue(dest, res);
                std::cout << "  " << dest << " = " << v1 << " " << token2 << " " << v2 << " = " << res << std::endl;
                continue;
            } else {
                
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

    if (!fir.params.empty() && static_cast<int>(fir.params.size()) != static_cast<int>(args.size())) {
        std::cerr << "Aviso: função '" << name << "' esperava " << fir.params.size()
                  << " args, recebeu " << args.size() << ".\n";
    }

    std::unordered_map<std::string, double> newScope;

    int limit = std::min(static_cast<int>(fir.params.size()), static_cast<int>(args.size()));
    for (int i = 0; i < limit; ++i) {
        newScope[fir.params[i]] = args[i];
    }

    callStack.push_back(newScope);

    auto retPair = executeLines(fir.body);

    callStack.pop_back();

    if (retPair.first) {
        return retPair.second;
    } else {
        
        return 0.0;
    }
}

void Interpreter::executeInstruction(const std::string& line) {
    
    std::string l = trim(line);
    if (l.empty()) return;
    
    std::vector<std::string> tmp{l};
    executeLines(tmp);
}

void Interpreter::execute() {
    std::cout << "\n=== EXECUÇÃO DO CÓDIGO ===\n";

    for (const auto &line : mainLines) {
        if (line.empty() || line.find("===") != std::string::npos) continue;
        std::cout << "Executando: " << line << std::endl;

        if (startsWith(line, "func_") || startsWith(line, "end_") || startsWith(line, "param ")) {
            continue;
        }

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
            if (!kv.first.empty() && kv.first[0] == 't') continue; 
            std::cout << kv.first << " = " << kv.second << std::endl;
        }
    } else {
        std::cout << "(nenhuma variável)\n";
    }
}
