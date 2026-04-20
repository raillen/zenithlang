# Checklist tools.* (Go/No-Go)

- Status: checklist inicial
- Data: 2026-04-20
- Escopo: bibliotecas oficiais `tools.*`

## Objetivo

Acompanhar a maturidade das bibliotecas `tools.*` sem misturar esse trabalho com a `stdlib`.

## Gate 0: Fronteira arquitetural

- [ ] `tools.*` documentado como namespace oficial
- [ ] `std.*` nao depende de `tools.*`
- [ ] `zt.*` pode depender de `tools.*`
- [ ] pasta `tools/decisions` criada
- [ ] primeira decision de fronteira aprovada
- [ ] criterio claro para mover algo entre `std` e `tools`

## Gate 1: tools.cli

- [ ] suporta comandos
- [ ] suporta subcomandos
- [ ] suporta flags booleanas
- [ ] suporta flags com valor
- [ ] suporta argumentos posicionais
- [ ] gera help automaticamente
- [ ] retorna erro tipado para input invalido
- [ ] suporta modo `--json` via integracao com `tools.output`
- [ ] behavior tests cobrindo help, erro e sucesso

## Gate 2: tools.output

- [ ] suporta formato humano
- [ ] suporta JSON
- [ ] suporta NDJSON ou formato streaming equivalente
- [ ] suporta `quiet`
- [ ] suporta `verbose`
- [ ] padroniza exit code
- [ ] nao mistura output de dados com logs de erro
- [ ] snapshots de output em testes

## Gate 3: tools.diagnostics

- [ ] suporta severidade
- [ ] suporta codigo de erro
- [ ] suporta `ACTION`
- [ ] suporta `WHY`
- [ ] suporta `NEXT`
- [ ] suporta span de arquivo quando aplicavel
- [ ] suporta sugestoes
- [ ] tem modo compacto
- [ ] tem modo detalhado
- [ ] testado com exemplos TDAH/dislexia-friendly

## Gate 4: tools.prompt

- [ ] confirmacao sim/nao
- [ ] input de texto
- [ ] senha mascarada
- [ ] selecao unica
- [ ] selecao multipla
- [ ] timeout ou cancelamento
- [ ] modo nao interativo
- [ ] erro claro quando prompt nao pode rodar em CI

## Gate 5: tools.progress

- [ ] etapas nomeadas
- [ ] spinner simples
- [ ] barra de progresso quando houver total conhecido
- [ ] modo sem animacao para CI
- [ ] logs agrupados
- [ ] duracao por etapa
- [ ] falha mostra etapa onde parou

## Gate 6: tools.table

- [ ] alinhamento por coluna
- [ ] truncamento por largura
- [ ] modo compacto
- [ ] modo markdown
- [ ] suporte a valores ausentes
- [ ] snapshots de tabela
- [ ] comportamento previsivel em terminal estreito

## Gate 7: tools.template

- [ ] placeholders simples
- [ ] erro para variavel ausente
- [ ] escape seguro
- [ ] renderizacao deterministica
- [ ] suporte a templates de arquivo
- [ ] usado por scaffold real

## Gate 8: comandos oficiais usando tools

- [ ] `zt doctor` usa `tools.diagnostics`
- [ ] `zt doctor` usa `tools.table` ou `tools.output`
- [ ] `zt new` usa `tools.cli`
- [ ] `zt new` usa `tools.prompt`
- [ ] `zt new` usa `tools.template`
- [ ] `zt bench` usa `tools.progress`
- [ ] `zt bench` usa `tools.output`

## Gate 9: documentacao

- [ ] ZDoc de `tools.cli`
- [ ] ZDoc de `tools.output`
- [ ] ZDoc de `tools.diagnostics`
- [ ] ZDoc de `tools.prompt`
- [ ] ZDoc de `tools.progress`
- [ ] ZDoc de `tools.table`
- [ ] ZDoc de `tools.template`
- [ ] exemplos pequenos por modulo
- [ ] tutorial criando uma CLI completa

## Gate 10: Go/No-Go v1

Para considerar `tools.*` v1 pronta:

- [ ] `tools.cli` estavel
- [ ] `tools.output` estavel
- [ ] `tools.diagnostics` estavel
- [ ] pelo menos 2 comandos `zt.*` usando `tools.*`
- [ ] snapshots de output no CI
- [ ] prompts nao travam CI
- [ ] docs suficientes para criar uma CLI do zero
- [ ] decision de compatibilidade publicada
