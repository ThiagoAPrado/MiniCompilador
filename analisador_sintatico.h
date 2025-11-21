#ifndef ANALISADOR_SINTATICO_H
#define ANALISADOR_SINTATICO_H

#include <vector>
#include <string>
#include <cstddef> 
#include "simbolo.h"

class AnalisadorSintatico {
private:
    std::vector<Token> tokens;
    size_t posicaoAtual;
    Token tokenAtual;
    
    void avancar();
    void consumir(TipoToken tipoEsperado, const std::string& mensagemErro);
    bool verificar(TipoToken tipo);
    
    // Regras da gramática
    void programa();
    void declaracao();
    void declaracaoFuncao();
    void declaracaoVariavel();
    void expressao();
    void termo();
    void fator();
    void potencia();
    void elemento();

public:
    AnalisadorSintatico(const std::vector<Token>& tokens);
    bool analisar();
    void mostrarErro(const std::string& mensagem);
};

#endif