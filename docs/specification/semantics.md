# Especificação Técnica: Semântica e Escopo

A camada semântica do Zenith é responsável por atribuir significado às estruturas sintáticas, garantindo que a resolução de nomes e o fluxo de dados sejam consistentes e seguros.

## 1. Resolução de Nomes e Símbolos

Zenith utiliza escopo léxico estático. A busca por um símbolo segue a hierarquia do bloco atual para os blocos pais, até atingir o escopo do módulo ou global.

### Símbolos de Módulo
- Símbolos marcados com pub são exportados e visíveis para outros módulos que importam o namespace.
- Símbolos sem pub são privados ao arquivo/módulo onde foram declarados.

### Shadowing
É permitido declarar uma variável com o mesmo nome de uma variável em um escopo superior. O compilador tratará a nova declaração como um símbolo distinto dentro do bloco atual.

---

## 2. Mutabilidade e Constantes

- var: Declara uma variável mutável. O valor pode ser alterado após a inicialização.
- const: Declara um valor imutável em tempo de compilação.
- global: Declara um símbolo no escopo global da VM, visível em todo o ecossistema (uso desencorajado, exceto para configurações de sistema).

---

## 3. Controle de Fluxo e Retorno

Zenith impõe regras estritas sobre o fluxo de execução para evitar comportamentos indefinidos.

### Retorno Obrigatório
Funções com tipo de retorno definido (ex: -> int) devem garantir que todos os caminhos de execução possíveis terminem em um statement return ou em um erro fatal (panic).

### Match Exhaustiveness
O statement match deve cobrir todos os casos possíveis de um Enum ou fornecer um caso padrão (case _ => ...) para garantir que nenhuma condição fique sem tratamento.

---

## 4. Reatividade (State Management)

Zenith possui suporte nativo para programação orientada a estado:

- state: Variáveis cujo valor dispara notificações quando alterado.
- computed: Valores derivados de um ou mais state, recalculados apenas quando as dependências mudam.
- watch: Blocos de código que executam automaticamente em resposta a mudanças de um state.

---
*Zenith Specification v0.3.0*
