#include "analisador_sintatico.h"
#include <iostream>
#include <stdexcept>

using namespace std;

AnalisadorSintatico::AnalisadorSintatico(const vector<Token>& t) 
    : tokens(t), posicaoAtual(0) {
    if (!tokens.empty()) {
        tokenAtual = tokens[0];
    } else {
        tokenAtual = Token(TipoToken::FIM_ARQUIVO, "");
    }
}

void AnalisadorSintatico::avancar() {
    posicaoAtual++;
    if (posicaoAtual < tokens.size()) {
        tokenAtual = tokens[posicaoAtual];
    } else {
        tokenAtual = Token(TipoToken::FIM_ARQUIVO, "");
    }
}

void AnalisadorSintatico::consumir(TipoToken tipoEsperado, const string& mensagemErro) {
    if (tokenAtual.tipo == tipoEsperado) {
        avancar();
    } else {
        mostrarErro(mensagemErro);
        throw runtime_error("Erro sintático");
    }
}

bool AnalisadorSintatico::verificar(TipoToken tipo) {
    return tokenAtual.tipo == tipo;
}

void AnalisadorSintatico::mostrarErro(const string& mensagem) {
    cout << "Erro sintático na linha " << tokenAtual.linha 
         << ", coluna " << tokenAtual.coluna << ": " 
         << mensagem << endl;
    cout << "Token encontrado: " << tokenAtual.valor << endl;
}

bool AnalisadorSintatico::analisar() {
    try {
        programa();
        cout << "Análise sintática concluída com sucesso!" << endl;
        return true;
    } catch (const exception& e) {
        cout << "Análise sintática falhou: " << e.what() << endl;
        return false;
    }
}

void AnalisadorSintatico::programa() {
    while (tokenAtual.tipo != TipoToken::FIM_ARQUIVO) {
        declaracao();
    }
}

void AnalisadorSintatico::declaracao() {
    if (verificar(TipoToken::FUNCAO)) {
        declaracaoFuncao();
    } else if (verificar(TipoToken::IDENTIFICADOR)) {
        declaracaoVariavel();
    } else {
        mostrarErro("Esperado declaração de função ou variável");
        throw runtime_error("Erro de declaração");
    }
}

void AnalisadorSintatico::declaracaoFuncao() {
    consumir(TipoToken::FUNCAO, "Esperado 'funcao'");
    consumir(TipoToken::IDENTIFICADOR, "Esperado nome da função");
    consumir(TipoToken::PARENTESE_ESQ, "Esperado '('");
    
    // Parâmetros
    if (!verificar(TipoToken::PARENTESE_DIR)) {
        consumir(TipoToken::IDENTIFICADOR, "Esperado parâmetro");
        while (verificar(TipoToken::VIRGULA)) {
            avancar();
            consumir(TipoToken::IDENTIFICADOR, "Esperado parâmetro após vírgula");
        }
    }
    
    consumir(TipoToken::PARENTESE_DIR, "Esperado ')'");
    consumir(TipoToken::ATRIBUICAO, "Esperado '='");
    expressao();
    consumir(TipoToken::PONTO_VIRGULA, "Esperado ';'");
}

void AnalisadorSintatico::declaracaoVariavel() {    
    consumir(TipoToken::IDENTIFICADOR, "Esperado nome da variável");
    consumir(TipoToken::ATRIBUICAO, "Esperado '='");
    expressao();
    consumir(TipoToken::PONTO_VIRGULA, "Esperado ';'");
}

void AnalisadorSintatico::expressao() {
    termo();
    while (verificar(TipoToken::MAIS) || verificar(TipoToken::MENOS)) {
        avancar();
        termo();
    }
}

void AnalisadorSintatico::termo() {
    fator();
    while (verificar(TipoToken::MULTIPLICACAO) || verificar(TipoToken::DIVISAO)) {
        avancar();
        fator();
    }
}

void AnalisadorSintatico::fator() {
    potencia();
    // Pode ser expandido para mais operadores
}

void AnalisadorSintatico::potencia() {
    elemento();
    while (verificar(TipoToken::POTENCIA)) {
        avancar();
        elemento();
    }
}

void AnalisadorSintatico::elemento() {
    if (verificar(TipoToken::IDENTIFICADOR)) {
        avancar();
        // Verifica se é chamada de função
        if (verificar(TipoToken::PARENTESE_ESQ)) {
            avancar();
            if (!verificar(TipoToken::PARENTESE_DIR)) {
                expressao();
                while (verificar(TipoToken::VIRGULA)) {
                    avancar();
                    expressao();
                }
            }
            consumir(TipoToken::PARENTESE_DIR, "Esperado ')'");
        }
    } else if (verificar(TipoToken::INTEIRO_LIT) || verificar(TipoToken::REAL_LIT)) {
        avancar();
    } else if (verificar(TipoToken::PARENTESE_ESQ)) {
        avancar();
        expressao();
        consumir(TipoToken::PARENTESE_DIR, "Esperado ')'");
    } else {
        mostrarErro("Esperado identificador, número ou '('");
        throw runtime_error("Erro de elemento");
    }
}