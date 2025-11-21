#include "analisador_semantico.h"
#include <iostream>

using namespace std;

AnalisadorSemantico::AnalisadorSemantico(const vector<Token>& t) : tokens(t) {
    entrarEscopo(); // Escopo global
}

void AnalisadorSemantico::entrarEscopo() {
    tabelaSimbolos.push_back(unordered_map<string, Simbolo>());
}

void AnalisadorSemantico::sairEscopo() {
    if (!tabelaSimbolos.empty()) {
        tabelaSimbolos.pop_back();
    }
}

bool AnalisadorSemantico::declararSimbolo(const Simbolo& simbolo) {
    if (tabelaSimbolos.empty()) return false;
    
    auto& escopoAtual = tabelaSimbolos.back();
    
    // Usar insert em vez de operator[] para evitar necessidade de construtor padrão
    auto result = escopoAtual.insert({simbolo.nome, simbolo});
    return result.second; // retorna true se inserção foi bem sucedida
}

Simbolo* AnalisadorSemantico::buscarSimbolo(const string& nome) {
    // Busca do escopo mais interno para o mais externo
    for (auto it = tabelaSimbolos.rbegin(); it != tabelaSimbolos.rend(); ++it) {
        auto found = it->find(nome);
        if (found != it->end()) {
            return &found->second;
        }
    }
    return nullptr;
}

TipoDado AnalisadorSemantico::inferirTipo(const Token& token) {
    if (token.tipo == TipoToken::INTEIRO_LIT) return TipoDado::INTEIRO;
    if (token.tipo == TipoToken::REAL_LIT) return TipoDado::REAL;
    return TipoDado::INDEFINIDO;
}

bool AnalisadorSemantico::analisar() {
    cout << "Iniciando análise semântica..." << endl;
    
    bool sucesso = true;
    
    // Primeiro: processar declarações de funções e seus parâmetros
    for (size_t i = 0; i < tokens.size(); i++) {
        // Processar declaração de função
        if (tokens[i].tipo == TipoToken::FUNCAO && i + 1 < tokens.size()) {
            if (tokens[i+1].tipo == TipoToken::IDENTIFICADOR) {
                string nomeFuncao = tokens[i+1].valor;
                Simbolo funcao(nomeFuncao, TipoDado::INDEFINIDO, true);
                if (declararSimbolo(funcao)) {
                    cout << "Função '" << nomeFuncao << "' declarada com sucesso" << endl;
                }
                
                // Processar parâmetros da função
                size_t j = i + 2;
                if (j < tokens.size() && tokens[j].tipo == TipoToken::PARENTESE_ESQ) {
                    j++;
                    while (j < tokens.size() && tokens[j].tipo != TipoToken::PARENTESE_DIR) {
                        if (tokens[j].tipo == TipoToken::IDENTIFICADOR) {
                            string param = tokens[j].valor;
                            Simbolo varParam(param, TipoDado::INDEFINIDO, false);
                            if (declararSimbolo(varParam)) {
                                cout << "Parâmetro '" << param << "' declarado para função '" << nomeFuncao << "'" << endl;
                            }
                        }
                        j++;
                    }
                }
            }
        }
    }
    
    // Segundo: processar declarações de variáveis
    for (size_t i = 0; i < tokens.size(); i++) {
        // Processar declaração de variável (identificador seguido de '=')
        if (tokens[i].tipo == TipoToken::IDENTIFICADOR && i + 2 < tokens.size()) {
            if (tokens[i+1].tipo == TipoToken::ATRIBUICAO) {
                // Verificar se não é um parâmetro de função
                bool ehParametroFuncao = false;
                if (i > 0) {
                    // Verificar se está dentro de uma declaração de função
                    for (size_t k = i; k > 0; k--) {
                        if (tokens[k].tipo == TipoToken::FUNCAO) {
                            ehParametroFuncao = true;
                            break;
                        }
                        if (tokens[k].tipo == TipoToken::ATRIBUICAO && tokens[k-1].tipo == TipoToken::PARENTESE_DIR) {
                            break; // Sai do escopo da função
                        }
                    }
                }
                
                if (!ehParametroFuncao) {
                    string nomeVar = tokens[i].valor;
                    TipoDado tipo = inferirTipo(tokens[i+2]);
                    
                    Simbolo var(nomeVar, tipo, false);
                    if (declararSimbolo(var)) {
                        cout << "Variável '" << nomeVar << "' declarada com sucesso" << endl;
                    }
                    
                    // Pular os tokens já processados
                    i += 2; // Já processamos identificador, =, e o valor
                }
            }
        }
    }
    
    // Terceiro: verificar usos de identificadores
    for (size_t i = 0; i < tokens.size(); i++) {
        if (tokens[i].tipo == TipoToken::IDENTIFICADOR) {
            string nome = tokens[i].valor;
            
            // Ignorar se for declaração de variável
            if (i + 1 < tokens.size() && tokens[i + 1].tipo == TipoToken::ATRIBUICAO) {
                // Verificar se não é parâmetro de função
                bool ehParametro = false;
                if (i > 0) {
                    for (size_t k = i; k > 0; k--) {
                        if (tokens[k].tipo == TipoToken::FUNCAO) {
                            ehParametro = true;
                            break;
                        }
                        if (tokens[k].tipo == TipoToken::ATRIBUICAO && tokens[k-1].tipo == TipoToken::PARENTESE_DIR) {
                            break;
                        }
                    }
                }
                if (!ehParametro) {
                    continue; // É declaração, pular
                }
            }
            
            // Ignorar se for nome de função em declaração
            if (i > 0 && tokens[i-1].tipo == TipoToken::FUNCAO) {
                continue;
            }
            
            // Verificar se é chamada de função
            if (i + 1 < tokens.size() && tokens[i + 1].tipo == TipoToken::PARENTESE_ESQ) {
                verificarChamadaFuncao(i);
            }
            // Verificar uso de variável
            else {
                verificarUsoVariavel(i);
            }
        }
    }
    
    cout << "Análise semântica concluída!" << endl;
    return sucesso;
}

void AnalisadorSemantico::verificarAtribuicao(size_t pos) {
    string nomeVar = tokens[pos].valor;
    Simbolo* simbolo = buscarSimbolo(nomeVar);
    
    if (!simbolo) {
        cout << "Erro semântico: Variável '" << nomeVar << "' não declarada (linha " 
             << tokens[pos].linha << ")" << endl;
    } else {
        cout << "Atribuição para variável '" << nomeVar << "' verificada (linha " 
             << tokens[pos].linha << ")" << endl;
    }
}

void AnalisadorSemantico::verificarChamadaFuncao(size_t pos) {
    string nomeFuncao = tokens[pos].valor;
    Simbolo* simbolo = buscarSimbolo(nomeFuncao);
    
    if (!simbolo || !simbolo->ehFuncao) {
        cout << "Erro semântico: Função '" << nomeFuncao << "' não declarada (linha " 
             << tokens[pos].linha << ")" << endl;
    } else {
        cout << "Chamada de função '" << nomeFuncao << "' verificada (linha " 
             << tokens[pos].linha << ")" << endl;
    }
}

void AnalisadorSemantico::verificarUsoVariavel(size_t pos) {
    string nomeVar = tokens[pos].valor;
    Simbolo* simbolo = buscarSimbolo(nomeVar);
    
    if (!simbolo) {
        cout << "Erro semântico: Variável '" << nomeVar << "' não declarada (linha " 
             << tokens[pos].linha << ")" << endl;
    } else {
        // Não mostrar mensagem para uso normal de variável para não poluir
        // cout << "Uso da variável '" << nomeVar << "' verificado (linha " 
        //      << tokens[pos].linha << ")" << endl;
    }
}