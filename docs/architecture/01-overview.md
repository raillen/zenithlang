# Arquitetura do Compilador Zenith

**Objetivo**: Explicar como o código Zenith é transformado em Lua.  
**Público-alvo**: Contribuidores e desenvolvedores que queiram entender os "internals".  
**Contexto**: Arquitetura v0.2

## Conteúdo Principal

O Zenith utiliza um pipeline de transpilação de passagem múltipla (multi-pass). O lema principal é: **O Frontend não sabe que o Lua existe**.

### O Pipeline de Compilação

1. **Lexer (`lexer.lua`)**: Transforma o fluxo de caracteres em um fluxo de tokens. Ele lida com comentários, strings interpoladas e palavras-chave.
2. **Parser (`parser.lua`)**: Utiliza o algoritmo **Pratt Parsing** (Top-Down Operator Precedence) para lidar com expressões e um Recursive Descent para declarações. O resultado é uma **AST (Abstract Syntax Tree)** pura.
3. **Binder (`binder.lua`)**: É o coração semântico. Ele percorre a AST para resolver nomes (escopos), tipos e validar as regras da linguagem. Ele gera uma "Bound Tree" ou anota a própria AST com símbolos e tipos.
4. **Codegen (`lua_codegen.lua`)**: Recebe a AST validada e emite o código Lua correspondente. Ele utiliza padrões de visita para cada tipo de nó.

## Decisões e Justificativas

- **Decisão**: Uso do Pratt Parser.
- **Motivo**: Permite definir precedência de operadores de forma muito elegante e extensível, facilitando a adição de novos operadores sem "inferno recursivo".
- **Decisão**: Separação total do Emitter.
- **Motivo**: Permite que futuramente o Zenith mude o target (ex: para C ou Bytecode nativo) sem precisar mexer na lógica semântica da linguagem.

## Exemplos Comentados

### Fluxo de um `match`:
1. `parser.lua` cria um nó `MATCH_STMT`.
2. `binder.lua` entra no `_bind_match_stmt`, cria um novo escopo de bloco e valida se os padrões casam com o tipo da expressão.
3. `lua_codegen.lua` recebe o nó, chama `_emit_match_stmt`, que por sua vez chama `_gen_destructure_logic` para cada braço do match.

## Impactos na Implementação
A arquitetura modular permite que erros sejam detectados o mais cedo possível. Se um erro ocorre no Binder, o Codegen nem sequer é iniciado, garantindo que nunca geremos código Lua inválido por erro de tipo no Zenith.

**Status**: Implementado (v0.2)
