#ifndef ANALISADOR_LEXICO_H
#define ANALISADOR_LEXICO_H

#include <string>
#include <vector>
#include <cstddef>
#include "simbolo.h"

class AnalisadorLexico {
private:
    std::string codigoFonte;
    size_t posicao;
    int linha;
    int coluna;
    char caractereAtual;
    
    void avancar();
    char olharAdiante();
    void pularEspacos();
    Token processarNumero();
    Token processarIdentificador();
    Token processarOperador();

public:
    AnalisadorLexico(const std::string& codigo);
    std::vector<Token> analisar();
    Token proximoToken();
    void reiniciar();
};

#endif