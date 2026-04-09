# Zenith Language — Hub de Implementação

> Versão de referência: v1.0-alpha  
> Target de saída: Lua 5.4  
> Paradigma: transpiler com frontend próprio

## Resumo rápido
O documento de implementação do Zenith foi reorganizado em módulos menores para facilitar leitura progressiva, revisão técnica e retomada após pausas.

## Quando usar
Comece aqui se você quer entender a arquitetura geral do compilador e decidir qual parte aprofundar em seguida.

## Pré-requisitos
- Entender a proposta geral da linguagem.
- Saber que o target atual é Lua 5.4.
- Ter interesse em internals do compilador.

## Tempo de leitura
3 a 4 minutos para escolher o próximo módulo.

## Como navegar

Use a trilha abaixo como índice:

1. **Visão geral e princípios**  
   Arquivo: `/docs-content/architecture/01-principles.md`  
   Foco: missão do projeto, critérios de clareza e regra de separação entre frontend e backend.

2. **Camadas e pipeline**  
   Arquivo: `/docs-content/architecture/02-layers-and-pipeline.md`  
   Foco: estrutura de pastas, responsabilidades de cada camada e fluxo de compilação.

3. **Frontend sintático**  
   Arquivo: `/docs-content/architecture/03-syntax-frontend.md`  
   Foco: `source/`, `diagnostics/`, tokens, lexer, AST e parser.

4. **Semântica e lowering**  
   Arquivo: `/docs-content/architecture/04-semantics-and-lowering.md`  
   Foco: símbolos, tipos, binding, validações e simplificação antes do backend.

5. **Tooling e testes**  
   Arquivo: `/docs-content/architecture/05-tooling-and-tests.md`  
   Foco: pipeline, sistema de projeto, CLI, testes, convenções e ordem de implementação.

## Trilha sugerida

- Se você vai propor uma feature nova, comece por **Visão geral e princípios**.
- Se você precisa localizar uma mudança arquitetural, leia **Camadas e pipeline**.
- Se a mudança envolve gramática ou parser, vá para **Frontend sintático**.
- Se a mudança envolve tipos, escopo ou desugaring, vá para **Semântica e lowering**.
- Se a mudança envolve build, CLI ou testes, vá para **Tooling e testes**.

## Regra de bolso

Antes de alterar o compilador, responda:

> esta mudança afeta forma, significado, simplificação ou emissão?

Essa pergunta normalmente aponta para o módulo certo e evita misturar responsabilidades.
