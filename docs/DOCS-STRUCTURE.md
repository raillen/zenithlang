# Docs Structure

> Regras de organizacao da documentacao Zenith.
> Status: atual.
> Superficie: interna e publica.

## Decisao

A documentacao e separada por publico-alvo.

Isso e a fonte de verdade para decidir onde um novo documento deve ficar.

## Camadas

### `docs/public/`

Conteudo que pode ir para o site da linguagem.

Inclui:

- primeiros passos;
- instalacao;
- trilhas de aprendizado;
- cookbook de problemas comuns;
- historia e manifesto publico;
- tutoriais;
- guias de uso;
- exemplos curtos;
- material de onboarding;
- guias de ferramentas para usuario.

Nao inclui:

- roadmap;
- checklist;
- relatorio;
- auditoria;
- debate de design;
- plano inacabado.

### `docs/reference/`

Conteudo consultavel e relativamente estavel.

Inclui:

- referencia curta de sintaxe;
- referencia de tipos;
- referencia de stdlib;
- diagnosticos;
- modelo de projeto;
- KB tecnica organizada.

Nao substitui `language/spec/` quando a pergunta e normativa para implementacao.

### `docs/internal/`

Conteudo de evolucao do repositorio.

Inclui:

- roadmaps;
- checklists;
- reports;
- governance;
- arquitetura interna;
- standards editoriais;
- arquivos historicos;
- rascunhos e materiais brutos.

### `docs/wiki/`

Fonte da GitHub Wiki.

Nao use `wiki_repo/` como fonte. Esse diretorio e temporario quando o workflow sincroniza a wiki.

### `language/spec/`

Especificacao normativa para implementacao.

Use quando uma regra precisa orientar parser, typechecker, runtime, stdlib ou backend.

### `language/decisions/`

Decisoes com contexto.

Use quando precisamos preservar:

- problema;
- alternativas;
- decisao;
- motivo;
- consequencias.

### Documentacao colocalizada

Alguns documentos tecnicos ficam perto do codigo.

Isso e permitido quando o documento e usado para manutencao direta do subsistema.

Exemplos:

- `compiler/*_MAP.md`;
- `runtime/c/README.md`;
- `stdlib/README.md`;
- `tests/README.md`;
- `tools/*/README.md`;
- `packages/*/README.md`.

O indice desses documentos fica em `docs/internal/architecture/codebase-map.md`.

## Tabela rapida

| Documento | Pasta |
| --- | --- |
| Guia para iniciante | `docs/public/` |
| Trilha de aprendizado | `docs/public/learn/` |
| Receita de problema comum | `docs/public/cookbook/` |
| Historia e manifesto | `docs/public/history-and-manifesto.md` |
| Tutorial | `docs/public/` |
| Guia VSCode | `docs/public/guides/` |
| Guia ZPM | `docs/public/tools/` |
| Referencia de sintaxe | `docs/reference/` |
| Referencia de CLI | `docs/reference/cli/` |
| Referencia da stdlib | `docs/reference/stdlib/` |
| Knowledge base | `docs/reference/zenith-kb/` |
| API gerada por ZDoc | `docs/reference/api/` |
| Roadmap | `docs/internal/planning/` |
| Checklist | `docs/internal/planning/` |
| Relatorio de release | `docs/internal/reports/release/` |
| Auditoria | `docs/internal/reports/audit/` |
| Baseline de qualidade | `docs/internal/governance/` |
| Padrao editorial | `docs/internal/standards/` |
| Template de pagina de usuario | `docs/internal/standards/user-doc-template.md` |
| Mapa tecnico colocalizado | perto do codigo + indice em `docs/internal/architecture/` |
| Spec normativa | `language/spec/` |
| Decision/RFC | `language/decisions/` ou `docs/internal/decisions/` |

## Cabecalho recomendado

Use este bloco no topo de documentos novos:

```md
> Audience: user | package-author | contributor | maintainer
> Status: current | draft | historical | archived
> Surface: public | reference | internal | spec
> Source of truth: yes | no
```

## Politica de duplicidade

Evite duplicar conteudo entre camadas.

Regra:

- `docs/public/` explica com exemplos;
- `docs/reference/` resume regra consultavel;
- `language/spec/` define regra normativa;
- `docs/internal/` registra plano, evidencia e historico.

Se um conteudo precisar aparecer no site e na spec, a versao publica deve apontar para a fonte normativa.

## Politica de arquivo bruto

Materiais brutos ficam em:

- `docs/internal/archive/`;
- `docs/internal/reports/raw/`;
- `reports/` quando forem saidas locais geradas.

Nao coloque material bruto em `docs/public/`.
