# Build & Run

Para compilar e executar o mini compilador, clone este repositório e acesse a pasta `./MiniCompilador`

Para realizar a operação, você precisará de:

- g++ com suporte ao C++17 ou maior
- (Recomendado) make para facilitar a compilação

Abaixo estão as instruções para instalar essas dependências.

## Requisitos Linux

No Linux, o uso do make é a forma preferida e mais simples: 

```bash
sudo apt update
sudo apt install g++ make
```

Verifique:

```bash
g++ --version
make --version
```

Execute:

```bash
make run
```

## Requisitos Windows

A forma oficialmente recomendada para Windows é usar MSYS2, pois oferece suporte completo a make e g++.

Instalação:

- Baixe o instalador: https://www.msys2.org/
- Abra MSYS2 MinGW64
- Instale as ferramentas:

```bash
pacman -Syu
pacman -S mingw-w64-x86_64-gcc make
```

## Run

Para compilar e executar o código, rode:

```bash
make run
```

O comando acima irá compilar todos os arquivos .cpp localizados na pasta src/ e executar o binário gerado que será armazenado em `./bin`  e `./build`.

Caso prefira executar manual, rode:

```
g++ -std=c++17 src/*.cpp -Iinclude -o mini_compilador
./mini_compilador
```

# Mini Compilador

Este projeto tem como finalidade a construção de um mini compilador capaz de reconhecer, analisar e traduzir uma linguagem simples de expressões matemáticas, com suporte a:

- variáveis
- operadores aritméticos
- chamadas de funções
- funções definidas pelo usuário
- escopo simples baseado na ordem de declaração

Este documento descreve a arquitetura do compilador, explicando cada etapa: `Lexer → Tokens → Parser → AST → Execução (futura).`

## Visão Geral

O projeto implementa um pequeno compilador/interpreter baseado em:

* **Lexer** (tokenização do código)
* **Parser** (análise sintática)
* **AST** (árvore sintática abstrata)
* **Pretty Print** da AST

O objetivo é permitir ler linhas de código, gerar tokens, construir uma árvore sintática e exibir sua estrutura.

Fluxo resumido:

```
Código fonte → Lexer → Tokens → Parser → AST → Impressão da AST
```

## 1. AST – Abstract Syntax Tree (`ast.h`)

A AST representa a estrutura lógica do código.

Todos os nós herdam de:

```cpp
struct Node {
    virtual ~Node() = default;
    virtual void prettyPrint(int indent=0) const = 0;
};
```

### **Tipos de nós**

| Nó             | Descrição                         |
| -------------- | --------------------------------- |
| `NumberNode`   | Representa números                |
| `VarNode`      | Variáveis                         |
| `BinaryOpNode` | Operações aritméticas (+ - * / ^) |
| `FuncCallNode` | Chamada de função                 |
| `AssignNode`   | Atribuição (x = expr)             |
| `FuncDeclNode` | Declaração de função              |

Cada nó possui `prettyPrint()` para exibição hierárquica.

## 2. Tokens (`token.h`)

Tokens são as unidades léxicas que o lexer produz e o parser consome.

### **Tipos principais de tokens**

| Tipo      | Significado            |
| --------- | ---------------------- |
| `ID`      | Identificadores        |
| `NUM`     | Números                |
| `FUNC`    | Palavra-chave `funcao` |
| `OP_ARIT` | + - * / ^              |
| `ATRIB`   | =                      |
| `LPAREN`  | (                      |
| `RPAREN`  | )                      |
| `COMMA`   | ,                      |
| `EOF`     | Fim do código          |

Tokens incluem linha e coluna para erros.

## 3. Lexer (`lexer.h` / `lexer.cpp`)

O Lexer transforma texto bruto em tokens.

### Responsabilidades:

✔ Ler caractere por caractere

✔ Ignorar espaços e quebras de linha

✔ Identificar números

✔ Identificar identificadores e palavra-chave `funcao`

✔ Produzir tokens de operadores e símbolos

✔ Identificar tokens inválidos

### Métodos importantes:

| Método             | Função                                  |
| ------------------ | --------------------------------------- |
| `tokenize()`       | Converte o código em um vetor de tokens |
| `peek()`           | Olha o caractere atual                  |
| `get()`            | Consome o caractere                     |
| `skipWhitespace()` | Ignora espaços em branco                |


## 4. Parser (`parser.h` / `parser.cpp`)

Recebe tokens do lexer e monta a AST.

### 🧠 **Gramática implementada**

```
program      → declaration | assignment | expression

declaration  → funcao ID "(" params ")" "=" expression
assignment   → ID "=" expression
expression   → term (("+" | "-") term)*
term         → power (("*" | "/") power)*
power        → factor ("^" power)?
factor       → NUM | ID | ID("(" args ")") | "(" expression ")"
```

### Principais métodos:

| Método              | O que faz                                            |
| ------------------- | ---------------------------------------------------- |
| `parseAll()`        | Analisa todas as instruções do código                |
| `parseProgram()`    | Decide se é declaração, atribuição ou expressão      |
| `parseExpression()` | Trata + e -                                          |
| `parseTerm()`       | Trata * e /                                          |
| `parsePower()`      | Trata ^ com associatividade à direita                |
| `parseFactor()`     | Números, variáveis, chamadas de função ou parênteses |
| `parseParameters()` | Parâmetros de função                                 |
| `parseArguments()`  | Argumentos de função                                 |


## 5. Arquivo principal (`main.cpp`)

Realiza o fluxo completo:

1. Lê múltiplas linhas do usuário
2. Envia o código para o Lexer, gerando tokens
3. Mostra os tokens gerados
4. Envia os tokens para o Parser, que constrói a AST
5. Executa o Parser (`parseAll()`)
6. Exibe a AST formatada com `prettyPrint()`
7. Exibe erros, se houver

### Leitura de múltiplas linhas

- Usuário digita várias linhas.
- Uma linha vazia encerra o input.
- Todo o conteúdo é armazenado no stringstream.

```cpp
std::cout << "Digite o código da linguagem (uma linha por vez, termine com linha vazia):\n";

std::stringstream buffer;
std::string linha;

while (true) {
    std::getline(std::cin, linha);
    if (linha.empty()) break;
    buffer << linha << '\n';
}

```

### Tokenização

```cpp
Lexer lexer(codigo);
auto tokens = lexer.tokenize();
```

O Lexer:

- percorre o código caractere por caractere
- reconhece palavras, números, operadores etc.
- gera uma lista de Tokens

O programa então imprime os tokens:

```cpp
for (auto &t : tokens) {
    std::cout << "line:" << t.line 
              << " col:" << t.column 
              << " " << tokenTypeToString(t.type)
              << "('" << t.value << "')\n";
}
```

### Parsing e AST

```cpp
Parser parser(tokens);
auto ast_list = parser.parseAll();
```

O Parser:

- consome os tokens
- valida a estrutura do código
- gera nós de AST (árvore sintática)

Depois imprime cada nó da AST:

```cpp
for (auto &ast : ast_list) {
    ast->prettyPrint();
}
```

### Tratamento de erros

Se o Parser encontrar algo inesperado (token inválido, sintaxe incorreta), o erro é mostrado ao usuário.

```cpp
catch (std::exception &e) {
    std::cerr << "Erro de parser: " << e.what() << "\n";
}
```

### Exemplos de Execução

#### Exemplo 1 - Atribuições simples

**Entrada:**

```php
x = 10
y = x + 1


```
_(linha vazia para finalizar)_

**Saída:**

```php
Código recebido:
x = 10
y = x + 1

Tokens:
line:1 col:1 ID('x')
line:1 col:3 ATRIB('=')
line:1 col:5 NUM('10')
line:2 col:1 ID('y')
line:2 col:3 ATRIB('=')
line:2 col:5 ID('x')
line:2 col:7 OP_ARIT('+')
line:2 col:9 NUM('1')
line:3 col:1 END_OF_FILE('')

AST:
Assign(x)
  Number(10)
Assign(y)
  BinaryOp(+)
    Var(x)
    Number(1)
```

#### Exemplo 2 - Três atribuições

**Entrada:**

```php
x = 10
y = x + 1
z = y * 2


```
_(linha vazia para finalizar)_

**Saída:**

```php
Código recebido:
x = 10
y = x + 1

Código recebido:
x = 10
y = x + 1
z = y * 2


Tokens:
line:1 col:1 ID('x')
line:1 col:3 ATRIB('=')
line:1 col:5 NUM('10')
line:2 col:1 ID('y')
line:2 col:3 ATRIB('=')
line:2 col:5 ID('x')
line:2 col:7 OP_ARIT('+')
line:2 col:9 NUM('1')
line:3 col:1 ID('z')
line:3 col:3 ATRIB('=')
line:3 col:5 ID('y')
line:3 col:7 OP_ARIT('*')
line:3 col:9 NUM('2')
line:4 col:1 EOF('')

AST:
Assign(x)
  Number(10)
Assign(y)
  BinaryOp(+)
    Var(x)
    Number(1)
Assign(z)
  BinaryOp(*)
    Var(y)
    Number(2)
```

#### Exemplo 3 - Função

**Entrada:**

```php
funcao soma(a, b) = a + b
soma(56 + 98)


```
_(linha vazia para finalizar)_

**Saída:**

```php
Código recebido:
funcao soma(a, b) = a + b
soma(56 + 98)


Tokens:
line:1 col:1 FUNC('funcao')
line:1 col:8 ID('soma')
line:1 col:12 LPAREN('(')
line:1 col:13 ID('a')
line:1 col:14 COMMA(',')
line:1 col:16 ID('b')
line:1 col:17 RPAREN(')')
line:1 col:19 ATRIB('=')
line:1 col:21 ID('a')
line:1 col:23 OP_ARIT('+')
line:1 col:25 ID('b')
line:2 col:1 ID('soma')
line:2 col:5 LPAREN('(')
line:2 col:6 NUM('56')
line:2 col:9 OP_ARIT('+')
line:2 col:11 NUM('98')
line:2 col:13 RPAREN(')')
line:3 col:1 EOF('')

AST:
FuncDecl(soma)
  Params:
    a
    b
  Body:
    BinaryOp(+)
      Var(a)
      Var(b)
FuncCall(soma)
  BinaryOp(+)
    Number(56)
    Number(98)
```

#### Exemplo 3 - Erro de sintaxe

**Entrada:**

```php
x = 10 +


```
_(linha vazia para finalizar)_

**Saída:**

```php
Tokens:
line:1 col:1 ID('x')
line:1 col:3 ATRIB('=')
line:1 col:5 NUM('10')
line:1 col:8 OP_ARIT('+')
line:2 col:1 END_OF_FILE('')

Erro de parser: Unexpected end of expression at line 1
```

#### Exemplo 4 - Token inválido

**Entrada:**

```php
x = 10 @ 2


```
_(linha vazia para finalizar)_

**Saída:**

```php
Tokens:
line:1 col:1 ID('x')
line:1 col:3 ATRIB('=')
line:1 col:5 NUM('10')
line:1 col:8 INVALID('@')
line:1 col:10 NUM('2')
line:2 col:1 END_OF_FILE('')

Erro de parser: Invalid token '@' at line 1
```