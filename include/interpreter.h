#ifndef INTERPRETER_H
#define INTERPRETER_H

#include <vector>
#include <string>
#include <unordered_map>
#include <iostream>
#include <utility>

struct FunctionIR {
    std::vector<std::string> params;  
    std::vector<std::string> body;    
};

class Interpreter {
private:

    std::unordered_map<std::string, double> variables;
    std::unordered_map<std::string, FunctionIR> functions;

    std::vector<std::string> mainLines;
    std::vector<std::unordered_map<std::string, double>> callStack;

    double getValue(const std::string& name);
    void setValue(const std::string& name, double value);
    void executeInstruction(const std::string& line);

    std::pair<bool, double> executeLines(const std::vector<std::string>& lines);

    double callFunction(const std::string& name, const std::vector<double>& args);

public:
    Interpreter(const std::vector<std::string>& lines);
    void execute();
    void printVariables() const;
};

#endif
