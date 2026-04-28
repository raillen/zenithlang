# Zenith Cookbook

> Audience: user
> Status: draft
> Surface: public
> Source of truth: no

## Objetivo

Responder perguntas práticas:

"Como faco isto em Zenith?"

O cookbook existe para transformar conceitos da linguagem em soluções diretas.

## Escopo

Inclui receitas para problemas comuns:

- ausencia sem `null`;
- erro sem exception;
- union segura;
- método abstrato;
- dispatch virtual;
- validação de entrada;
- conversão para texto;
- lookup seguro;
- falhas intencionais;
- testes legiveis.

Não inclui:

- spec normativa;
- roadmap;
- debate histórico de design;
- features futuras como se fossem atuais.

## Dependencias

- Upstream:
  - `docs/internal/planning/documentation-roadmap-v1.md`
  - `docs/internal/standards/user-doc-template.md`
- Downstream:
  - `docs/public/README.md`
  - `docs/reference/language/`
- Código/Testes relacionados:
  - `tests/behavior/MATRIX.md`

## Conteudo principal

Formato padrão de receita:

1. Problema.
2. Resposta curta.
3. Código recomendado.
4. Explicacao.
5. Erro comum.
6. Quando não usar.

Receitas iniciais recomendadas:

| Receita | Use quando |
| --- | --- |
| `absence-without-null.md` | um valor pode não existir |
| `errors-without-exceptions.md` | uma operação pode falhar e o chamador deve decidir |
| `safe-union-with-enum.md` | um valor pode ter formatos diferentes |
| `abstract-methods-with-trait.md` | varios tipos precisam cumprir o mesmo contrato |
| `virtual-dispatch-with-dyn.md` | valores de tipos diferentes precisam ser chamados pelo mesmo contrato |
| `partial-class-with-apply.md` | você quer separar métodos sem fragmentar os dados |
| `callable-delegate.md` | você quer passar uma funcao como valor |
| `zenith-equivalents-from-other-languages.md` | você conhece o conceito em outra linguagem e quer o equivalente Zenith |
| `intentional-failure-builtins.md` | você precisa falhar de modo claro com `check`, `todo` ou `unreachable` |
| `stdlib-practical-recipes.md` | você quer receitas pequenas da stdlib ligadas a exemplos executaveis |

## Validação

Antes de publicar uma receita:

```powershell
python tools/check_docs_paths.py
git diff --check
```

Exemplos devem compilar ou ser marcados como ilustrativos.

## Histórico de atualização

- 2026-04-25: índice inicial do cookbook.
- 2026-04-26: adicionada receita de equivalentes Zenith para conceitos de outras linguagens.
- 2026-04-26: adicionada receita de falhas intencionais com `check`, `todo` e `unreachable`.
- 2026-04-28: adicionadas receitas praticas da stdlib.
