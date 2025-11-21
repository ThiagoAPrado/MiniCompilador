#ifndef ANALISADOR_SEMANTICO_H
#define ANALISADOR_SEMANTICO_H

#include <vector>
#include <unordered_map>
#include <string>
#include <cstddef>
#include "simbolo.h"

class AnalisadorSemantico {
private:
    std::vector<std::unordered_map<std::string, Simbolo>> tabelaSimbolos;
    std::vector<Token> tokens;
    
    void entrarEscopo();
    void sairEscopo();
    bool declararSimbolo(const Simbolo& simbolo);
    Simbolo* buscarSimbolo(const std::string& nome);
    TipoDado inferirTipo(const Token& token);
    void verificarUsoVariavel(size_t pos);

public:
    AnalisadorSemantico(const std::vector<Token>& tokens);
    bool analisar();
    void verificarAtribuicao(size_t pos); 
    void verificarChamadaFuncao(size_t pos); 
};

#endif