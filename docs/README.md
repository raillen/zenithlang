# Documentacao Zenith

> Mapa principal da documentacao do projeto.
> Status: atual.
> Superficie: indice.

## Objetivo

Separar documentacao por publico-alvo.

Isso evita que usuarios leiam checklists internos como se fossem guia da linguagem,
e evita que contribuidores usem tutoriais como fonte normativa de implementacao.

## Pastas principais

| Pasta | Publico | Uso |
| --- | --- | --- |
| `docs/public/` | usuarios | site, guias, tutoriais e primeiros passos |
| `docs/reference/` | usuarios avancados e autores de packages | regras consultaveis, KB e referencias estaveis |
| `docs/internal/` | mantenedores e contribuidores | roadmaps, checklists, reports, governance e arquitetura interna |
| `docs/wiki/` | publico | fonte sincronizada para GitHub Wiki |
| `language/spec/` | mantenedores e implementacao | especificacao normativa bruta da linguagem |
| `language/decisions/` | mantenedores e implementacao | decisoes de linguagem com contexto historico |

## Fontes atuais

| Necessidade | Fonte atual |
| --- | --- |
| Aprender a usar | `docs/public/` |
| Consultar regra curta | `docs/reference/` |
| Roadmap ativo | `docs/internal/planning/roadmap-v4.md` |
| Checklist ativo | `docs/internal/planning/checklist-v4.md` |
| Especificacao normativa | `language/spec/README.md` |
| Decisoes de linguagem | `language/decisions/README.md` |
| Relatorios e evidencia | `docs/internal/reports/` |
| Saidas locais de teste/build | `reports/` |
| Mapas tecnicos colocalizados | `docs/internal/architecture/codebase-map.md` |
| Plano de melhoria da documentacao | `docs/internal/planning/documentation-roadmap-v1.md` |

## Regra simples

- Se ensina usuario a usar Zenith, coloque em `docs/public/`.
- Se define uma regra consultavel, coloque em `docs/reference/`.
- Se planeja, registra risco, evidencia ou implementacao, coloque em `docs/internal/`.
- Se define comportamento normativo do compilador, mantenha em `language/spec/`.
- Se registra motivo e contexto de decisao, mantenha em `language/decisions/`.
- Se documenta manutencao direta de um subsistema, pode ficar perto do codigo e ser indexado em `docs/internal/architecture/codebase-map.md`.

## Ordem de leitura para mantenedores

1. `README.md` na raiz.
2. `docs/DOCS-STRUCTURE.md`.
3. `docs/internal/planning/roadmap-v4.md`.
4. `docs/internal/planning/checklist-v4.md`.
5. `language/spec/README.md`.
6. `language/decisions/README.md`.
7. `docs/internal/architecture/codebase-map.md`.
8. `compiler/CODE_MAP.md`.
9. `docs/internal/reports/README.md`.

## Padrao editorial

Use `docs/internal/standards/documentation-style-guide.md`.
Para paginas de usuario, use tambem `docs/internal/standards/user-doc-template.md`.

Resumo:

- frases curtas;
- titulos previsiveis;
- exemplos pequenos;
- status explicito;
- atual, historico e futuro sempre separados.

## Validacao

Antes de fechar uma reorganizacao documental, rode:

```powershell
python tools/check_docs_paths.py
git diff --check
```
