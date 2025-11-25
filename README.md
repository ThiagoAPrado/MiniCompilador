# Mini Compilador --- Linguagem de Expressões Funcionais

Este projeto implementa um mini compilador completo, capaz de analisar, validar, gerar código intermediário e interpretar uma linguagem simples de expressões funcionais matemáticas.

``` 
Código → Lexer → Tokens → Parser → AST → Semântico → Codegen → Execução
```
-   Tokenizar o código (Lexer)
-   Analisar sintaticamente e construir a AST (Parser)
-   Verificar erros semânticos (Semantic)
-   Gerar instruções de uma linguagem intermediária simples (Codegen)
-   Executar o código gerado (Interpreter)

### Suporta:

- Declaração de variáveis
- Atribuições
- Expressões aritméticas (+ - * / ^)
- Precedência e associatividade corretas
- Declaração de funções
- Chamadas de função
- Escopo e parâmetros
- Verificação semântica
- Geração de código intermediário (três endereços)
- Execução real com interpretador

# Build & Run

## Requisitos

- **C++17** ou superior
- **g++** no Linux
- **MinGW** ou **Clang** no Windows

## Compilar

Entre no diretório do projeto:

``` bash
cd MiniCompilador
```

### Windows

``` bash
g++ -std=c++20 -Wall -Wextra -O2 -Iinclude src/*.cpp -o MiniCompilador.exe
```

``` bash
MiniCompilador.exe
```

### Linux

``` bash
g++ -std=c++20 -Wall -Wextra -O2 -Iinclude src/*.cpp -o MiniCompilador
```

``` bash
./MiniCompilador
```

# Definição da gramática

## Declaração de variáveis

A declaração de variáveis segue o padrão: `nome_variável = expressão`

### Exemplo

```
x = 10      // declaração da variável 'x'
y = 15.8    // declaração da variável 'y'
z = x + y   // declaração da variável 'z' e atribuição dos valores
```

## Declaração e chamada de Funções

A declaração de funções segue o padrão: `funcao nome_função(parâmetros) = expressão`

### Exemplo

```
funcao soma(a, b) = a + b  // declaração da função com sua expressão
resultado = soma(10, 15)   // chamada da função em uma variável
```

# Arquitetura do Compilador

O projeto utiliza cinco grandes módulos:

`Lexer → Parser → AST → Semantic Analyzer → CodeGenerator → Interpreter`


## 1. Lexer (`lexer.h` / `lexer.cpp`)

O Lexer transforma texto bruto em tokens.

- Ler caractere por caractere
- Ignorar espaços e quebras de linha
- Identificar números `int` e `float`
- Identificar identificadores e palavra-chave `funcao`
- Criar tokens de operadores e símbolos
- Criar token de erro para caracteres inválidos

### Métodos importantes:

| Método             | Função                                  |
| ------------------ | --------------------------------------- |
| `tokenize()`       | Converte o código em um vetor de tokens |
| `peek()`           | Olha o caractere atual                  |
| `get()`            | Consome o caractere                     |
| `skipWhitespace()` | Ignora espaços em branco                |

## 2. Tokens (`token.h`)

Tokens são as unidades léxicas que o lexer produz e o parser consome. Cada token possui:

- Tipo (`TokenType`)
- O valor lido
- Linha e coluna (para erros)
- Suporte completo a erros léxicos

### Tipos principais de tokens:

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
| `INVALID` | Caracter inválido      |

## 3. Parser (`parser.h` / `parser.cpp`)

### Implementa a gramática:

```
program      → declaration | assignment | expression
declaration  → funcao ID "(" params ")" "=" expression
assignment   → ID "=" expression
expression   → term (("+" | "-") term)*
term         → power (("*" | "/") power)*
power        → factor ("^" power)?
factor       → NUM | ID | chamada | "(" expressão ")"
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
| `parseParameters()` | Parâmetros de função na delaração                    |
| `parseArguments()`  | Argumentos de função em chamadas                     |

## 4. AST – Abstract Syntax Tree (`ast.h`)

### Nós principais:

| Nó             | Descrição                         |
| -------------- | --------------------------------- |
| `NumberNode`   | Representa números                |
| `VarNode`      | Variáveis                         |
| `BinaryOpNode` | Operações aritméticas (+ - * / ^) |
| `FuncCallNode` | Chamada de função                 |
| `AssignNode`   | Atribuição (x = expr)             |
| `FuncDeclNode` | Declaração de função              |

### Todos os nós possuem

``` cpp
prettyPrint(int indent = 0)
```

## 5. Análise Semântica (`semantic.h` / `semantic.cpp`)

Valida:

- variáveis declaradas
- funções declaradas
- aridade das funções
- duplicação de parâmetros
- validade de operações

Usa tabela de símbolos com suporte a escopo.

### Verifica:

- uso de variáveis não declaradas (runtime)
- funções não declaradas
- funções chamadas com número errado de argumentos
- parâmetros duplicados
- duplicação de nome de função

### Estruturas utilizadas:

- tabela de funções { nome → parâmetros / corpo }
- escopo de parâmetros
- detecção de aridade

Se algo estiver errado, lança `std::runtime_error`

## 6. Geração de Código (`codegen.h` / `codegen.cpp`)

Gera código intermediário em três endereços, por exemplo:

``` ini 
func soma:
  param a
  param b
  t0 = a + b
  return t0
end_soma:

arg0 = 10
arg1 = 20
t1 = call soma 2
x = t1
```
obs: `t1 = call soma 2` o número 2 representa a quantidade de argumentos retornados para `t1`

### Suporta:

- criação de temporários (`t0`, `t1`, `...`)
- tabelas de funções
- parâmetros
- argumentos (`arg0`, `arg1`, `...`)
- chamadas de função
- instruções atribuição

### Métodos principais

| Método                   | Função                                                                       |
| ------------------------ | ---------------------------------------------------------------------------- |
| `generateCode(astList)`  | Percorre a lista de nós da AST e gera o código intermediário correspondente. |
| `emit(line)`             | Adiciona uma linha de código gerado à lista interna de instruções.           |
| `newTemp()`              | Cria e retorna o nome de um novo registrador/variável temporária exclusivo.  |
| `printCode()`            | Exibe no console todas as linhas de código intermediário já emitidas.        |
| `getCodeLines()`         | Retorna a lista completa de instruções geradas para uso externo.             |

## 7. Interpretador (`interpreter.h` / `interpreter.cpp`)

Executa o código intermediário linha por linha.

### Funcionalidades:

- Armazena variáveis e temporários
- Armazena parâmetros e argumentos
- Executa chamadas de função simulando "pilha" de retorno
- Executa expressões aritméticas
- Produz saída final da execução

### Exemplo:

``` ini
Executando: arg0 = 10
  arg0 = 10
Executando: arg1 = 20
  arg1 = 20
Executando: arg2 = 30
  arg2 = 30
Executando: t2 = call a 3
  t0 = 10 + 20 = 30
  t1 = 30 + 30 = 60
  t2 = 60 (call a)
Executando: x = t2
  x = 60
```

No final, imprime todas as variáveis conhecidas.

# 8. Exemplos de entradas

## Exemplo 1

### Entrada:

```
funcao soma(a, b) = a + b
x = soma(2, 3)
```

### Tokens:

```
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
line:2 col:1 ID('x')
line:2 col:3 ATRIB('=')
line:2 col:5 ID('soma')
line:2 col:9 LPAREN('(')
line:2 col:10 NUM('2')
line:2 col:11 COMMA(',')
line:2 col:13 NUM('3')
line:2 col:14 RPAREN(')')
line:3 col:1 EOF('')
```

### AST (Abstract Syntax Tree):

```
FuncDecl(soma)
  Params:
    a
    b
  Body:
    BinaryOp(+)
      Var(a)
      Var(b)
Assign(x)
  FuncCall(soma)
    Number(2 : int)
    Number(3 : int)
```

### Semântica:

```
Análise semântica OK!
```

### Código intermediário:

```
func_soma:
  param a
  param b
  t0 = a + b
  return t0
end_soma:

  arg0 = 2
  arg1 = 3
  t1 = call soma 2
x = t1
```

### Execução do código:

```
Executando: arg0 = 2
  arg0 = 2
Executando: arg1 = 3
  arg1 = 3
Executando: t1 = call soma 2
  t0 = 2 + 3 = 5
  t1 = 5 (call soma)
Executando: x = t1
  x = 5
```

### Variáveis:

```
arg0 = 2
arg1 = 3
x = 5
```

## Exemplo 2

### Entrada:

```
x = 2
y = x + z
```

### Tokens:

```
line:1 col:1 ID('x')
line:1 col:3 ATRIB('=')
line:1 col:5 NUM('2')
line:2 col:1 ID('y')
line:2 col:3 ATRIB('=')
line:2 col:5 ID('x')
line:2 col:7 OP_ARIT('+')
line:2 col:9 ID('z')
line:3 col:1 EOF('')
```

### AST (Abstract Syntax Tree):

```
Assign(x)
  Number(2 : int)
Assign(y)
  BinaryOp(+)
    Var(x)
    Var(z)
```

### Semântica:

```
Erro semântico: variável 'z' não declarada.
```

## Exemplo 3

### Entrada:

```
x = 2 ^ 3 ^ 2
```

### Tokens:

```
line:1 col:1 ID('x')
line:1 col:3 ATRIB('=')
line:1 col:5 NUM('2')
line:1 col:7 OP_ARIT('^')
line:1 col:9 NUM('3')
line:1 col:11 OP_ARIT('^')
line:1 col:13 NUM('2')
line:2 col:1 EOF('')
```

### AST (Abstract Syntax Tree):

```
Assign(x)
  BinaryOp(^)
    Number(2 : int)
    BinaryOp(^)
      Number(3 : int)
      Number(2 : int)
```

### Semântica:

```
Análise semântica OK!
```

### Código intermediário:

```
  t0 = 3 ^ 2
  t1 = 2 ^ t0
x = t1
```

### Execução do código:

```
Executando: t0 = 3 ^ 2
  t0 = 3 ^ 2 = 9
Executando: t1 = 2 ^ t0
  t1 = 2 ^ 9 = 512
Executando: x = t1
  x = 512
```

### Variáveis:

```
x = 512
```