# Documentacao Zenith

Este diretorio e o mapa principal da documentacao do projeto.

Objetivo:

- deixar claro onde cada tipo de informacao vive;
- reduzir duplicidade entre roadmap, checklist, specs e relatorios;
- manter leitura de baixa friccao para pessoas com TDAH e dislexia.

## Fontes atuais

| Necessidade | Fonte atual |
| --- | --- |
| Roadmap ativo | `docs/planning/roadmap-v4.md` |
| Checklist ativo | `docs/planning/checklist-v4.md` |
| Especificacao da linguagem | `language/spec/README.md` |
| Decisoes de linguagem | `language/decisions/README.md` |
| Wiki publicavel | `docs/wiki/` |
| Pendencias corretivas atuais | `reports/pending-language-issues-current.md` |
| Guia de extensao VS Code | `docs/guides/editor-vscode.md` |

Regra simples:

- Se o documento define comportamento da linguagem, use `language/`.
- Se o documento define plano de execucao, use `docs/planning/`.
- Se o documento registra evidencia ou fechamento, use `docs/reports/`.
- Se o arquivo e saida gerada por teste, build ou benchmark, use `reports/`.

## Estrutura

- `docs/governance/`
  Politicas de qualidade, baseline e aceite.
- `docs/guides/`
  Guias praticos e material de onboarding.
- `docs/licensing/`
  Licenciamento, contribuicao e marca.
- `docs/planning/`
  Roadmaps, checklists e planos por ciclo.
- `docs/reports/`
  Relatorios curados, auditorias e evidencia de release.
- `docs/standards/`
  Padroes editoriais e regras de documentacao.
- `docs/tools/`
  Guias de ferramentas do ecossistema.
- `docs/wiki/`
  Conteudo fonte para publicar na wiki.
- `docs/zenith-kb/`
  Base curta de conhecimento por area.

## Ordem de leitura recomendada

1. `README.md` na raiz.
2. `docs/planning/roadmap-v4.md`.
3. `docs/planning/checklist-v4.md`.
4. `language/spec/README.md`.
5. `language/decisions/README.md`.
6. `compiler/CODE_MAP.md`.
7. `docs/reports/README.md`.

## Regra para evitar bagunca

- Nao crie novo roadmap se o ciclo ativo ainda e o mesmo.
- Nao duplique pendencias em varios relatorios.
- Nao versione cache, build, log, screenshot temporario ou pacote instalado.
- Nao use `wiki_repo/` na raiz. O workflow cria esse diretorio temporariamente; a fonte real e `docs/wiki/`.

## Padrao editorial

Use `docs/standards/documentation-style-guide.md`.

Resumo:

- frases curtas;
- titulos previsiveis;
- exemplos pequenos;
- uma decisao por secao;
- status explicito: atual, historico, gerado ou proposta.
