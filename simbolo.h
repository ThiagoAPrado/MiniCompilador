#ifndef SIMBOLO_H
#define SIMBOLO_H

#include <string>
#include <vector>

enum class TipoDado {
    INTEIRO,
    REAL,
    INDEFINIDO
};

enum class TipoToken {
    // Palavras-chave
    FUNCAO, VARIAVEL, ATRIBUICAO, IGUAL,
    
    // Operadores
    MAIS, MENOS, MULTIPLICACAO, DIVISAO, POTENCIA,
    
    // Delimitadores
    PARENTESE_ESQ, PARENTESE_DIR, VIRGULA, PONTO_VIRGULA,
    
    // Identificadores e literais
    IDENTIFICADOR, INTEIRO_LIT, REAL_LIT,
    
    // Fim de arquivo
    FIM_ARQUIVO,
    
    // Erro
    ERRO
};

struct Token {
    TipoToken tipo;
    std::string valor;
    int linha;
    int coluna;
    
    Token(TipoToken t = TipoToken::ERRO, std::string v = "", int l = 0, int c = 0)
        : tipo(t), valor(v), linha(l), coluna(c) {}
};

struct Simbolo {
    std::string nome;
    TipoDado tipo;
    bool ehFuncao;
    std::vector<TipoDado> parametros;
    
    // Construtor padrão necessário para unordered_map
    Simbolo() : nome(""), tipo(TipoDado::INDEFINIDO), ehFuncao(false) {}
    
    // Construtor com parâmetros
    Simbolo(const std::string& n, TipoDado t, bool func = false)
        : nome(n), tipo(t), ehFuncao(func) {}
};

#endif