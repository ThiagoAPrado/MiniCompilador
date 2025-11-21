#ifndef GERADOR_CODIGO_H
#define GERADOR_CODIGO_H

#include <vector>
#include <string>
#include "simbolo.h"

struct InstrucaoTresEnderecos {
    std::string resultado;
    std::string operando1;
    std::string operador;
    std::string operando2;
    
    InstrucaoTresEnderecos(const std::string& res, const std::string& op1, 
                          const std::string& op, const std::string& op2)
        : resultado(res), operando1(op1), operador(op), operando2(op2) {}
};

class GeradorCodigo {
private:
    std::vector<InstrucaoTresEnderecos> codigoIntermediario;
    int contadorTemp;
    
public:
    GeradorCodigo();
    
    std::string novaTemp();
    void gerarAtribuicao(const std::string& var, const std::string& expr);
    void gerarOperacao(const std::string& var, const std::string& op1, 
                      const std::string& op, const std::string& op2);
    void gerarChamadaFuncao(const std::string& var, const std::string& funcao, 
                           const std::vector<std::string>& args);
    void exibirCodigo();
    std::vector<InstrucaoTresEnderecos> obterCodigo();
};

#endif