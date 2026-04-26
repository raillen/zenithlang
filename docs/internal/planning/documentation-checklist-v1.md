# Zenith Documentation Checklist v1

> Audience: maintainer
> Status: proposed
> Surface: internal
> Source of truth: yes

## Objetivo

Executar o roadmap de documentacao com entregas verificaveis.

A meta e produzir uma documentacao completa para usuario, com leitura facil para
pessoas com TDAH e dislexia.

## Escopo

Este checklist cobre:

- estrutura publica de aprendizado;
- manual completo da linguagem;
- cookbook;
- referencia completa;
- matriz de cobertura documental;
- padrao editorial acessivel.

## Dependencias

- Upstream:
  - `docs/internal/planning/documentation-roadmap-v1.md`
  - `docs/DOCS-STRUCTURE.md`
  - `docs/internal/standards/documentation-style-guide.md`
- Downstream:
  - `docs/public/`
  - `docs/reference/`
  - `docs/wiki/`
- Codigo/Testes relacionados:
  - `tools/check_docs_paths.py`
  - `tests/behavior/MATRIX.md`

## Conteudo principal

## D0 - Estrutura e padrao

- [x] Criar `docs/public/learn/`.
- [x] Criar `docs/public/cookbook/`.
- [x] Criar `docs/reference/language/feature-matrix.md`.
- [x] Criar template de pagina de usuario.
- [ ] Criar template de receita do cookbook.
- [x] Atualizar `docs/public/README.md`.
- [ ] Atualizar `docs/reference/README.md`.
- [x] Atualizar `docs/DOCS-STRUCTURE.md`.

Criterio de aceite:

- [ ] Um novo documento tem lugar claro antes de ser escrito.

## D1 - Manual de aprendizado

- [x] Primeiro programa.
- [x] Forma de arquivo.
- [x] Namespace e import.
- [x] Constantes e variaveis.
- [ ] Tipos primitivos.
- [x] Funcoes.
- [ ] Controle de fluxo.
- [ ] Texto e `fmt`.
- [ ] Listas e mapas.
- [x] `optional`.
- [x] `result`.
- [x] `?`.
- [ ] Structs.
- [ ] Traits e `apply`.
- [ ] Enums com payload.
- [ ] Generics e constraints.
- [ ] Callables/delegates.
- [ ] Closures/lambdas suportadas.
- [ ] `lazy<T>`.
- [ ] `dyn<Trait>`.
- [ ] Where contracts.
- [ ] Testes.
- [ ] Formatter.
- [ ] Diagnosticos.

Criterio de aceite:

- [ ] Cada pagina ensina um conceito com exemplos pequenos e limites atuais.

## D2 - Cookbook de problemas comuns

- [x] Como representar ausencia sem `null`.
- [x] Como representar erro sem exception.
- [x] Como fazer union segura com `enum` com payload.
- [x] Como fazer metodo abstrato com `trait`.
- [x] Como fazer dispatch virtual com `dyn<Trait>`.
- [x] Como separar metodos sem partial class usando `apply`.
- [ ] Como validar entrada.
- [ ] Como converter para texto.
- [ ] Como interpolar texto.
- [ ] Como fazer lookup seguro em list/map.
- [ ] Como usar `match` com `optional`.
- [ ] Como usar `match` com `result`.
- [ ] Como escolher entre `?`, `match` e helper.
- [ ] Como escrever API publica pequena.
- [ ] Como escrever testes legiveis.
- [x] Como passar uma funcao como valor.

Criterio de aceite:

- [ ] Cada receita tem problema, resposta curta, codigo, explicacao, erro comum e limite.

## D3 - Referencia completa da linguagem

- [ ] Expandir `syntax.md`.
- [ ] Expandir `types.md`.
- [ ] Expandir `modules-and-visibility.md`.
- [ ] Expandir `functions-and-control-flow.md`.
- [ ] Expandir `errors-and-results.md`.
- [ ] Criar referencia de `struct`, `trait`, `apply` e `dyn`.
- [ ] Criar referencia de `enum` e `match`.
- [ ] Criar referencia de generics e constraints.
- [ ] Criar referencia de callables, closures, lambdas e lazy.
- [ ] Criar referencia de contracts e diagnostics.
- [ ] Linkar cada pagina com spec e decisions relevantes.

Criterio de aceite:

- [ ] Uma pessoa consegue consultar qualquer feature atual sem abrir `language/decisions`.

## D4 - Matriz de cobertura documental

- [ ] Listar features atuais a partir de `tests/behavior/MATRIX.md`.
- [ ] Listar features de spec a partir de `language/spec/README.md`.
- [ ] Marcar cobertura: guia, referencia, cookbook, spec, teste.
- [ ] Marcar status: `complete`, `partial`, `missing`, `future`, `historical`.
- [ ] Priorizar `missing` e `partial`.

Criterio de aceite:

- [ ] Lacunas de documentacao ficam visiveis em uma tabela unica.

## D5 - Acessibilidade e ergonomia

- [ ] Revisar paginas para paragrafos curtos.
- [ ] Adicionar "quando usar" no topo de cada feature.
- [ ] Adicionar "quando nao usar" quando houver risco de abuso.
- [ ] Separar exemplos validos e invalidos.
- [ ] Adicionar comparacoes com outras linguagens onde reduz confusao.
- [ ] Evitar jargao sem definicao.
- [ ] Evitar blocos longos sem subtitulo.
- [ ] Garantir que feature futura esteja marcada como futura.

Criterio de aceite:

- [ ] A documentacao pode ser lida em blocos pequenos sem perder contexto.

## D6 - Validacao

- [ ] Rodar `python tools/check_docs_paths.py`.
- [ ] Rodar `git diff --check`.
- [ ] Conferir exemplos contra behavior tests quando possivel.
- [ ] Marcar exemplos ilustrativos quando ainda nao forem executaveis.
- [ ] Criar relatorio de fechamento em `docs/internal/reports/`.

## Validacao

Este checklist fecha apenas quando:

- o manual publico cobre a linguagem atual;
- a referencia e completa para consulta;
- o cookbook responde os problemas comuns;
- a matriz mostra cobertura documental real;
- nao ha feature atual importante escondida apenas em decisoes ou specs.

## Historico de atualizacao

- 2026-04-25: checklist inicial criado para a revisao ampla da documentacao.
