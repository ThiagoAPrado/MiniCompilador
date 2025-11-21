#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include "analisador_lexico.h"
#include "analisador_sintatico.h"
#include "analisador_semantico.h"
#include "gerador_codigo.h"

using namespace std;

string lerArquivo(const string& nomeArquivo) {
    ifstream arquivo(nomeArquivo);
    if (!arquivo.is_open()) {
        cerr << "Erro: Não foi possível abrir o arquivo " << nomeArquivo << endl;
        return "";
    }
    
    stringstream buffer;
    buffer << arquivo.rdbuf();
    return buffer.str();
}

string lerEntradaUsuario() {
    cout << "Digite seu código (linha por linha). Para finalizar, digite uma linha vazia:\n" << endl;
    cout << "Exemplos de comandos válidos:" << endl;
    cout << "  x = 10;                      # Declara variável" << endl;
    cout << "  funcao soma(a, b) = a + b;   # Declara função" << endl;
    cout << "  resultado = soma(5, 3) * 2;  # Usa função e variáveis\n" << endl;
    cout << "Comece a digitar seu código:\n" << endl;
    
    string codigo;
    string linha;
    int numeroLinha = 1;
    
    while (true) {
        cout << numeroLinha << "> ";
        getline(cin, linha);
        
        // Se a linha estiver vazia, finaliza a entrada
        if (linha.empty()) {
            if (numeroLinha == 1) {
                cout << "Nenhum código digitado. Voltando ao menu." << endl;
                return "";
            }
            break;
        }
        
        // Adiciona a linha ao código
        codigo += linha + "\n";
        numeroLinha++;
    }
    
    return codigo;
}

void processarCodigo(const string& codigoFonte, const string& descricao = "CÓDIGO") {
    cout << "\n=== PROCESSANDO " << descricao << " ===" << endl;
    
    if (codigoFonte.empty()) {
        cout << "Código vazio! Nada para processar." << endl;
        return;
    }
    
    cout << "\nCódigo fonte:\n" << codigoFonte << endl;
    
    // Análise Léxica
    cout << "\n--- ANÁLISE LÉXICA ---" << endl;
    AnalisadorLexico lexico(codigoFonte);
    vector<Token> tokens = lexico.analisar();
    
    // Mostrar apenas os primeiros tokens para não poluir a tela
    int tokensMostrados = 0;
    for (const auto& token : tokens) {
        if (tokensMostrados < 15) { // Mostrar apenas os primeiros 15 tokens
            cout << "Token: " << token.valor << " | Tipo: " << static_cast<int>(token.tipo) 
                 << " | Linha: " << token.linha << " | Coluna: " << token.coluna << endl;
            tokensMostrados++;
        }
        if (token.tipo == TipoToken::FIM_ARQUIVO) break;
    }
    if (tokens.size() > 15) {
        cout << "... e mais " << (tokens.size() - 15) << " tokens" << endl;
    }
    
    // Análise Sintática
    cout << "\n--- ANÁLISE SINTÁTICA ---" << endl;
    AnalisadorSintatico sintatico(tokens);
    bool sintaxeOk = sintatico.analisar();
    
    if (sintaxeOk) {
        // Análise Semântica
        cout << "\n--- ANÁLISE SEMÂNTICA ---" << endl;
        AnalisadorSemantico semantico(tokens);
        bool semanticaOk = semantico.analisar();
        
        // Geração de Código
        if (semanticaOk) {
            cout << "\n--- GERAÇÃO DE CÓDIGO ---" << endl;
            GeradorCodigo gerador;

            // 1. Primeiro: Gerar código para funções
            for (size_t i = 0; i < tokens.size(); i++) {
                if (tokens[i].tipo == TipoToken::FUNCAO && i + 1 < tokens.size()) {
                    string nomeFuncao = tokens[i+1].valor;
                    cout << "Função '" << nomeFuncao << "' detectada - gerando código..." << endl;
                    
                    // Encontrar o corpo da função (após o '=')
                    size_t j = i + 1;
                    while (j < tokens.size() && tokens[j].tipo != TipoToken::ATRIBUICAO) {
                        j++;
                    }
                    if (j < tokens.size()) {
                        j++; // Pular o '='
                        
                        // Gerar operações do corpo da função
                        while (j < tokens.size() && tokens[j].tipo != TipoToken::PONTO_VIRGULA) {
                            if (tokens[j].tipo == TipoToken::IDENTIFICADOR && j + 2 < tokens.size()) {
                                if (tokens[j+1].tipo == TipoToken::MULTIPLICACAO && 
                                    (tokens[j+2].tipo == TipoToken::IDENTIFICADOR || tokens[j+2].tipo == TipoToken::INTEIRO_LIT)) {
                                    
                                    string temp = gerador.novaTemp();
                                    gerador.gerarOperacao(temp, tokens[j].valor, "*", tokens[j+2].valor);
                                    cout << "  Operação na função: " << temp << " = " << tokens[j].valor << " * " << tokens[j+2].valor << endl;
                                }
                                else if (tokens[j+1].tipo == TipoToken::MAIS && 
                                         (tokens[j+2].tipo == TipoToken::IDENTIFICADOR || tokens[j+2].tipo == TipoToken::INTEIRO_LIT)) {
                                    
                                    string temp = gerador.novaTemp();
                                    gerador.gerarOperacao(temp, tokens[j].valor, "+", tokens[j+2].valor);
                                    cout << "  Operação na função: " << temp << " = " << tokens[j].valor << " + " << tokens[j+2].valor << endl;
                                }
                                else if (tokens[j+1].tipo == TipoToken::POTENCIA && 
                                         (tokens[j+2].tipo == TipoToken::IDENTIFICADOR || tokens[j+2].tipo == TipoToken::INTEIRO_LIT)) {
                                    
                                    string temp = gerador.novaTemp();
                                    gerador.gerarOperacao(temp, tokens[j].valor, "^", tokens[j+2].valor);
                                    cout << "  Operação na função: " << temp << " = " << tokens[j].valor << " ^ " << tokens[j+2].valor << endl;
                                }
                            }
                            j++;
                        }
                    }
                }
            }

            // 2. Segundo: Gerar atribuições de variáveis globais
            for (size_t i = 0; i < tokens.size(); i++) {
                if (tokens[i].tipo == TipoToken::IDENTIFICADOR && i + 2 < tokens.size()) {
                    if (tokens[i+1].tipo == TipoToken::ATRIBUICAO) {
                        string nomeVar = tokens[i].valor;
                        string valor = tokens[i+2].valor;
                        
                        // Verificar se é uma atribuição simples (não dentro de função)
                        bool dentroDeFuncao = false;
                        for (size_t k = i; k > 0; k--) {
                            if (tokens[k].tipo == TipoToken::FUNCAO) {
                                dentroDeFuncao = true;
                                break;
                            }
                            if (tokens[k].tipo == TipoToken::PONTO_VIRGULA) {
                                break;
                            }
                        }
                        
                        if (!dentroDeFuncao && (tokens[i+2].tipo == TipoToken::INTEIRO_LIT || tokens[i+2].tipo == TipoToken::REAL_LIT)) {
                            gerador.gerarAtribuicao(nomeVar, valor);
                            cout << "Atribuição: " << nomeVar << " = " << valor << endl;
                        }
                    }
                }
            }

            // 3. Terceiro: Gerar operações em expressões de variáveis globais
            for (size_t i = 0; i < tokens.size(); i++) {
                if (tokens[i].tipo == TipoToken::IDENTIFICADOR && i + 1 < tokens.size()) {
                    // Verificar se é uma expressão: var = expr op expr
                    if (tokens[i+1].tipo == TipoToken::ATRIBUICAO && i + 3 < tokens.size()) {
                        string varDestino = tokens[i].valor;
                        
                        // Verificar padrão: var = id op num
                        if (tokens[i+2].tipo == TipoToken::IDENTIFICADOR && 
                            (tokens[i+3].tipo == TipoToken::MAIS || 
                             tokens[i+3].tipo == TipoToken::MULTIPLICACAO ||
                             tokens[i+3].tipo == TipoToken::POTENCIA) &&
                            i + 4 < tokens.size() && 
                            (tokens[i+4].tipo == TipoToken::INTEIRO_LIT || tokens[i+4].tipo == TipoToken::REAL_LIT)) {
                            
                            string operador = "";
                            switch (tokens[i+3].tipo) {
                                case TipoToken::MAIS: operador = "+"; break;
                                case TipoToken::MULTIPLICACAO: operador = "*"; break;
                                case TipoToken::POTENCIA: operador = "^"; break;
                                default: break;
                            }
                            
                            if (!operador.empty()) {
                                string temp = gerador.novaTemp();
                                gerador.gerarOperacao(temp, tokens[i+2].valor, operador, tokens[i+4].valor);
                                gerador.gerarAtribuicao(varDestino, temp);
                                cout << "Expressão: " << varDestino << " = " << tokens[i+2].valor << " " << operador << " " << tokens[i+4].valor << endl;
                            }
                        }
                        // Verificar padrão: var = id op id
                        else if (tokens[i+2].tipo == TipoToken::IDENTIFICADOR && 
                                 (tokens[i+3].tipo == TipoToken::MAIS || 
                                  tokens[i+3].tipo == TipoToken::MULTIPLICACAO ||
                                  tokens[i+3].tipo == TipoToken::POTENCIA) &&
                                 i + 4 < tokens.size() && 
                                 tokens[i+4].tipo == TipoToken::IDENTIFICADOR) {
                            
                            string operador = "";
                            switch (tokens[i+3].tipo) {
                                case TipoToken::MAIS: operador = "+"; break;
                                case TipoToken::MULTIPLICACAO: operador = "*"; break;
                                case TipoToken::POTENCIA: operador = "^"; break;
                                default: break;
                            }
                            
                            if (!operador.empty()) {
                                string temp = gerador.novaTemp();
                                gerador.gerarOperacao(temp, tokens[i+2].valor, operador, tokens[i+4].valor);
                                gerador.gerarAtribuicao(varDestino, temp);
                                cout << "Expressão: " << varDestino << " = " << tokens[i+2].valor << " " << operador << " " << tokens[i+4].valor << endl;
                            }
                        }
                    }
                }
            }

            // Exibir código intermediário UMA ÚNICA VEZ
            cout << "\n=== CÓDIGO INTERMEDIÁRIO (TRÊS ENDEREÇOS) ===" << endl;
            gerador.exibirCodigo();
            
            cout << "\n=== COMPILAÇÃO CONCLUÍDA COM SUCESSO! ===" << endl;
        } else {
            cout << "\n=== COMPILAÇÃO FALHOU NA ANÁLISE SEMÂNTICA ===" << endl;
        }
    } else {
        cout << "\n=== COMPILAÇÃO FALHOU NA ANÁLISE SINTÁTICA ===" << endl;
    }
}

void demonstrarExemplo() {
    string codigoExemplo = 
        "funcao soma(a, b) = a + b;\n"
        "x = 10;\n"
        "y = 20.5;\n"
        "z = soma(x, y) * 2;\n"
        "resultado = z ^ 2;";
    
    processarCodigo(codigoExemplo, "EXEMPLO DE DEMONSTRAÇÃO");
}

void processarArquivo(const string& nomeArquivo) {
    cout << "Processando arquivo: " << nomeArquivo << endl;
    
    string codigoFonte = lerArquivo(nomeArquivo);
    if (codigoFonte.empty()) {
        return;
    }
    
    processarCodigo(codigoFonte, "ARQUIVO: " + nomeArquivo);
}

void entradaInterativa() {
    cout << "=== ENTRADA INTERATIVA ===" << endl;
    string codigo = lerEntradaUsuario();
    processarCodigo(codigo, "ENTRADA DO USUÁRIO");
}

int main() {
    cout << "Mini Compilador - Linguagem de Expressões Funcionais" << endl;
    cout << "====================================================" << endl;
    
    int opcao;
    
    while (true) {
        cout << "\nEscolha uma opção:" << endl;
        cout << "1. Usar arquivo de entrada" << endl;
        cout << "2. Demonstrar com exemplo" << endl;
        cout << "3. Entrada interativa (digitar código)" << endl;
        cout << "4. Sair" << endl;
        cout << "Opção: ";
        cin >> opcao;
        cin.ignore(); // Limpar buffer
        
        switch (opcao) {
            case 1: {
                string nomeArquivo;
                cout << "Digite o nome do arquivo: ";
                getline(cin, nomeArquivo);
                processarArquivo(nomeArquivo);
                break;
            }
            case 2:
                demonstrarExemplo();
                break;
            case 3:
                entradaInterativa();
                break;
            case 4:
                cout << "Saindo..." << endl;
                return 0;
            default:
                cout << "Opção inválida!" << endl;
                break;
        }
        
        cout << "\nPressione Enter para continuar...";
        cin.get();
    }
    
    return 0;
}