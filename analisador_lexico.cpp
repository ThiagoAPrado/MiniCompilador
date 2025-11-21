#include "analisador_lexico.h"
#include <cctype>
#include <unordered_map>

using namespace std;

AnalisadorLexico::AnalisadorLexico(const string& codigo) 
    : codigoFonte(codigo), posicao(0), linha(1), coluna(1) {
    if (!codigoFonte.empty()) {
        caractereAtual = codigoFonte[0];
    }
}

void AnalisadorLexico::avancar() {
    if (posicao < codigoFonte.length()) {
        posicao++;
        coluna++;
        
        if (posicao < codigoFonte.length()) {
            caractereAtual = codigoFonte[posicao];
            if (caractereAtual == '\n') {
                linha++;
                coluna = 1;
            }
        } else {
            caractereAtual = '\0';
        }
    }
}

char AnalisadorLexico::olharAdiante() {
    if (posicao + 1 < codigoFonte.length()) {
        return codigoFonte[posicao + 1];
    }
    return '\0';
}

void AnalisadorLexico::pularEspacos() {
    while (isspace(caractereAtual)) {
        avancar();
    }
}

Token AnalisadorLexico::processarNumero() {
    string valor;
    bool temPonto = false;
    int linhaInicio = linha;
    int colunaInicio = coluna;
    
    while (isdigit(caractereAtual) || caractereAtual == '.') {
        if (caractereAtual == '.') {
            if (temPonto) break; // Dois pontos - erro
            temPonto = true;
        }
        valor += caractereAtual;
        avancar();
    }
    
    return Token(temPonto ? TipoToken::REAL_LIT : TipoToken::INTEIRO_LIT, 
                valor, linhaInicio, colunaInicio);
}

Token AnalisadorLexico::processarIdentificador() {
    string valor;
    int linhaInicio = linha;
    int colunaInicio = coluna;
    
    while (isalnum(caractereAtual) || caractereAtual == '_') {
        valor += caractereAtual;
        avancar();
    }
    
    static unordered_map<string, TipoToken> palavrasChave = {
        {"funcao", TipoToken::FUNCAO},
    };
    
    auto it = palavrasChave.find(valor);
    if (it != palavrasChave.end()) {
        return Token(it->second, valor, linhaInicio, colunaInicio);
    }
    
    return Token(TipoToken::IDENTIFICADOR, valor, linhaInicio, colunaInicio);
}

Token AnalisadorLexico::processarOperador() {
    int linhaInicio = linha;
    int colunaInicio = coluna;
    char op = caractereAtual;
    avancar();
    
    switch (op) {
        case '+': return Token(TipoToken::MAIS, "+", linhaInicio, colunaInicio);
        case '-': return Token(TipoToken::MENOS, "-", linhaInicio, colunaInicio);
        case '*': return Token(TipoToken::MULTIPLICACAO, "*", linhaInicio, colunaInicio);
        case '/': return Token(TipoToken::DIVISAO, "/", linhaInicio, colunaInicio);
        case '^': return Token(TipoToken::POTENCIA, "^", linhaInicio, colunaInicio);
        case '=': return Token(TipoToken::ATRIBUICAO, "=", linhaInicio, colunaInicio);
        case '(': return Token(TipoToken::PARENTESE_ESQ, "(", linhaInicio, colunaInicio);
        case ')': return Token(TipoToken::PARENTESE_DIR, ")", linhaInicio, colunaInicio);
        case ',': return Token(TipoToken::VIRGULA, ",", linhaInicio, colunaInicio);
        case ';': return Token(TipoToken::PONTO_VIRGULA, ";", linhaInicio, colunaInicio);
        default: return Token(TipoToken::ERRO, string(1, op), linhaInicio, colunaInicio);
    }
}

Token AnalisadorLexico::proximoToken() {
    pularEspacos();
    
    if (posicao >= codigoFonte.length()) {
        return Token(TipoToken::FIM_ARQUIVO, "", linha, coluna);
    }
    
    if (isdigit(caractereAtual)) {
        return processarNumero();
    }
    
    if (isalpha(caractereAtual) || caractereAtual == '_') {
        return processarIdentificador();
    }
    
    return processarOperador();
}

vector<Token> AnalisadorLexico::analisar() {
    vector<Token> tokens;
    reiniciar();
    
    Token token = proximoToken();
    while (token.tipo != TipoToken::FIM_ARQUIVO) {
        tokens.push_back(token);
        token = proximoToken();
    }
    tokens.push_back(token); // Adiciona FIM_ARQUIVO
    
    return tokens;
}

void AnalisadorLexico::reiniciar() {
    posicao = 0;
    linha = 1;
    coluna = 1;
    if (!codigoFonte.empty()) {
        caractereAtual = codigoFonte[0];
    }
}