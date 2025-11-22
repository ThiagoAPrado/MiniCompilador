#ifndef INTERPRETER_H
#define INTERPRETER_H

#include <vector>
#include <string>
#include <unordered_map>
#include <iostream>

class Interpreter {
private:
    std::unordered_map<std::string, double> variables;
    std::vector<std::string> codeLines;
    
    double getValue(const std::string& name);
    void setValue(const std::string& name, double value);
    void executeInstruction(const std::string& line);
    
public:
    Interpreter(const std::vector<std::string>& lines);
    void execute();
    void printVariables() const;
};

#endif