# Zenith Implementation: Visão Geral e Princípios

## Resumo rápido
Este módulo define o posicionamento do compilador Zenith e as regras de clareza que devem orientar qualquer decisão de arquitetura. Ele explica por que a linguagem privilegia previsibilidade, separação de camadas e leitura linear acima de esperteza sintática.

## Quando usar
Leia este documento antes de propor novas features, reorganizar a arquitetura do compilador ou discutir o que o Zenith deve priorizar como linguagem.

## Pré-requisitos
- Conhecer a proposta geral do Zenith.
- Entender, em alto nível, que o target atual é Lua 5.4.
- Estar confortável com a ideia de compilador em múltiplas fases.

## Tempo de leitura
8 a 10 minutos.

## Exemplo mínimo
```text
source (.zt)
  -> syntax tree
  -> bound tree
  -> lowered IR
  -> lua output
```

## O que o Zenith está tentando ser

Zenith é uma linguagem de alto nível, estaticamente analisada e transpilada para Lua. A meta não é apenas “deixar Lua mais bonita”, mas oferecer uma linguagem legível, explícita, estável visualmente e cognitivamente acessível para scripting, automação, jogos, ensino e interfaces.

Essa proposta exige um equilíbrio claro entre:

- clareza;
- expressividade;
- previsibilidade;
- baixo atrito cognitivo;
- consistência arquitetural;
- facilidade de tooling.

## O que o Zenith não deve perseguir

O projeto não deve competir por:

- microconcisão;
- sintaxe excessivamente simbólica;
- metaprogramação extremamente livre no curto prazo;
- múltiplos paradigmas concorrentes sem uma organização simples de explicar.

Também não deve se reduzir a:

- uma camada sintática rasa sobre Lua;
- uma linguagem educacional simplista;
- um experimento “bonito”, mas difícil de manter.

## Clareza acima de esperteza sintática

Toda feature nova deve ser avaliada com a pergunta:

> o programador precisa lembrar regras demais ao usar isso?

O Zenith deve preferir construções que:

- sejam rapidamente reconhecíveis;
- tenham significado visual claro;
- reduzam inferência implícita;
- favoreçam leitura linear;
- permitam interrupção e retomada sem perda do contexto mental.

Essa diretriz importa especialmente para:

- TDAH, porque diminui o custo de retomar o fluxo;
- dislexia, porque reduz densidade simbólica e confusão entre formas parecidas.

## Princípios cognitivos que afetam a arquitetura

O design da linguagem e das ferramentas deve ajudar a pessoa a:

- localizar rapidamente onde está;
- entender onde blocos começam e terminam;
- voltar depois de uma pausa sem reprocessar o arquivo inteiro;
- confiar que estilos equivalentes terão aparência previsível.

Por isso o Zenith deve favorecer:

- blocos claramente delimitados;
- baixa oscilação visual entre formas equivalentes;
- palavras-chave reconhecíveis;
- boa separação espacial;
- mensagens de erro específicas e contextualizadas.

## Princípio de separação de camadas

O compilador Zenith é dividido em representações internas separadas:

1. **Syntax Tree**: o que o usuário escreveu.
2. **Bound Tree**: o que o código significa.
3. **Lua IR / Lua AST**: o que será emitido.

O corolário é simples:

> o frontend não sabe que Lua existe; só o backend conhece Lua.

Misturar essas camadas é o atalho que mais rapidamente degrada a clareza do projeto. Um nó semântico não deve “saber” como será emitido. Um emissor não deve fazer resolução de nomes.

## Escopo prático do projeto

No estado atual, Zenith deve ser particularmente bom para:

- ensino progressivo de programação;
- scripting local;
- automação;
- prototipagem de lógica;
- jogos e ferramentas sobre ecossistemas Lua;
- aplicações orientadas a estado.

Pode crescer, sem contradizer a filosofia principal, para:

- UI declarativa;
- bibliotecas de domínio específico;
- ferramentas educacionais;
- apps que dependem de estado reativo.

## Regra de ouro para contribuições

Uma mudança arquitetural só é válida quando melhora ao mesmo tempo:

- a explicabilidade da linguagem;
- a previsibilidade da implementação;
- a qualidade do tooling;
- a progressão cognitiva da documentação.

Se uma feature for poderosa, mas aumentar demais o número de exceções mentais, ela está contrariando a proposta central do Zenith.
