#include "gerador_codigo.h"
#include <iostream>

using namespace std;

GeradorCodigo::GeradorCodigo() : contadorTemp(0) {}

string GeradorCodigo::novaTemp() {
    return "t" + to_string(contadorTemp++);
}

void GeradorCodigo::gerarAtribuicao(const string& var, const string& expr) {
    codigoIntermediario.push_back(InstrucaoTresEnderecos(var, expr, "", ""));
}

void GeradorCodigo::gerarOperacao(const string& var, const string& op1, 
                                 const string& op, const string& op2) {
    codigoIntermediario.push_back(InstrucaoTresEnderecos(var, op1, op, op2));
}

void GeradorCodigo::gerarChamadaFuncao(const string& var, const string& funcao, 
                                      const vector<string>& args) {
    string argumentos;
    for (const auto& arg : args) {
        argumentos += arg + " ";
    }
    codigoIntermediario.push_back(InstrucaoTresEnderecos(var, funcao, "call", argumentos));
}

void GeradorCodigo::exibirCodigo() {
    // REMOVER o cabeçalho daqui - só mostrar instruções
    for (size_t i = 0; i < codigoIntermediario.size(); i++) {
        const auto& instr = codigoIntermediario[i];
        cout << i << ": " << instr.resultado << " = ";
        if (instr.operador.empty()) {
            cout << instr.operando1;
        } else if (instr.operador == "call") {
            cout << instr.operando1 << "(" << instr.operando2 << ")";
        } else {
            cout << instr.operando1 << " " << instr.operador << " " << instr.operando2;
        }
        cout << endl;
    }
}

vector<InstrucaoTresEnderecos> GeradorCodigo::obterCodigo() {
    return codigoIntermediario;
}