# Status Atual do ROADMAP_CORE

Documento derivado da leitura de [docs/ROADMAP_CORE.md](C:/Users/raillen.DESKTOP-99RJ5M6/Documents/Projetos/zenith-lang-v2/docs/ROADMAP_CORE.md) cruzada com o estado real do core da linguagem em 2026-04-09.

## Resumo

- A Fase 1 ja tem bastante base implementada.
- O maior passivo real esta nas Fases 2, 3 e 4.
- A P0 atual do core e fechar `enum` generico + `Optional/Outcome` end-to-end, porque isso impacta a prelude e o operador `?`.

## Atualizacao 2026-04-09

- P0 concluida no core: `enum` generico, `Optional/Outcome`, prelude e operador `?` ja fecham a pipeline principal.
- P1 iniciou pela trilha de deep desugaring: `where` saiu do codegen e passou a ser reescrito no lowerer.
- O `match` com `..resto` voltou a executar corretamente apos alinhar lowering + runtime de `slice`.
- O binder agora entende melhor `#expr` e `obj[i..j]`, o que destravou tipagem de slicing e dos binds usados pelo `match`.
- `match` com padrao de tipo (`case text:`) agora funciona para unions/aliases no core atual.
- Async/await da sintaxe atual foi consolidado com testes focados para funcao async, lambda async e awaits encadeados.

## Status da P1

- A P1 foi fechada no escopo atual do core da linguagem.
- Isso inclui: unions/aliases genericos em uso real, constraints profundas em acesso a membros genericos, deep desugaring de `match` e `where`, e a pipeline atual de async/await.
- Observacao importante: a runtime async continua sincrona por design da implementacao atual; o que foi fechado aqui foi a robustez do modelo atual do core, nao integracao com event loop externo.

## Matriz

| Item | Status | Bloqueador principal | Prioridade |
| --- | --- | --- | --- |
| F1. Lambdas | Quase pronto | Sintaxe, binder e codegen existem; falta consolidacao e estabilizacao geral | P2 |
| F1. Destruturacao em parametros | Parcial | A base existe, mas ainda depende de fechamento completo na pipeline | P2 |
| F1. Spread em expressoes | Quase pronto | Runtime e codegen existem; falta consolidacao e cobertura de casos | P2 |
| F1. Type aliases avancados | Concluido no core atual | `type`/`union` genericos e `match` tipado ja fecham os casos centrais do core | P1 |
| F2. Enums com dados | Parcial | Membros com payload existem, mas `enum` generico ainda nao fecha corretamente | P0 |
| F2. Generic constraints profundas | Concluido no core atual | Constraints em acesso a membros e chamadas genericas centrais ja estao cobertas | P1 |
| F2. Slicing de colecoes | Parcial | Ja existe `zt.slice`, mas ainda nao cobre o escopo prometido no roadmap | P2 |
| F2. Option/Result types | Quebrado | A ideia existe, mas ainda bloqueia o carregamento da prelude e o fluxo de `?` | P0 |
| F3. Deep desugaring na IR | Concluido no core atual | `match` e `where` ja baixam pela trilha principal do lowerer | P1 |
| F3. Transparent async/await | Concluido no core atual | A pipeline atual esta estavel; a runtime continua sincrona por design | P1 |
| F3. Blocos `native lua` | Ausente | Nao foi encontrada implementacao efetiva | P2 |
| F3. Trait `Iterable` | Ausente | `for-in` ainda esta acoplado a `list`/`range` | P2 |
| F4. ZPM / `.ztproj` | Parcial | Ja ha parser de projeto, mas nao ha gerenciamento real de dependencia externa | P2 |
| F4. Stdlib | Parcial | Hoje existem `core`, `io` e `math`, mas ainda faltam `std.os` e `std.json` | P2 |
| F4. Self-hosting | Ausente | Ainda nao ha partes do compilador reescritas em Zenith | P3 |
| F4. Auditoria de estabilidade (+500 casos) | Ausente | A suite ainda esta longe disso e o agregador de testes ainda mascara falhas reais | P1 |

## Ordem sugerida

1. Fechar P0: `enum` generico + `Optional/Outcome` + prelude + operador `?`.
2. Completar P2: ergonomia restante, `Iterable`, ZPM e stdlib.
3. Deixar P3 para depois da linguagem estabilizada.

## Observacoes fora do roadmap

- O binder ainda tinha um bug basico em declaracoes tipadas simples, mantendo simbolos como `any`.
- O runner agregado de testes ainda pode reportar sucesso falso.
- O CLI ainda nao estava alinhado com a pipeline completa do core para casos como prelude + lowering.
