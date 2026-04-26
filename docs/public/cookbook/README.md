# Zenith Cookbook

> Audience: user
> Status: draft
> Surface: public
> Source of truth: no

## Objetivo

Responder perguntas praticas:

"Como faco isto em Zenith?"

O cookbook existe para transformar conceitos da linguagem em solucoes diretas.

## Escopo

Inclui receitas para problemas comuns:

- ausencia sem `null`;
- erro sem exception;
- union segura;
- metodo abstrato;
- dispatch virtual;
- validacao de entrada;
- conversao para texto;
- lookup seguro;
- falhas intencionais;
- testes legiveis.

Nao inclui:

- spec normativa;
- roadmap;
- debate historico de design;
- features futuras como se fossem atuais.

## Dependencias

- Upstream:
  - `docs/internal/planning/documentation-roadmap-v1.md`
  - `docs/internal/standards/user-doc-template.md`
- Downstream:
  - `docs/public/README.md`
  - `docs/reference/language/`
- Codigo/Testes relacionados:
  - `tests/behavior/MATRIX.md`

## Conteudo principal

Formato padrao de receita:

1. Problema.
2. Resposta curta.
3. Codigo recomendado.
4. Explicacao.
5. Erro comum.
6. Quando nao usar.

Receitas iniciais recomendadas:

| Receita | Use quando |
| --- | --- |
| `absence-without-null.md` | um valor pode nao existir |
| `errors-without-exceptions.md` | uma operacao pode falhar e o chamador deve decidir |
| `safe-union-with-enum.md` | um valor pode ter formatos diferentes |
| `abstract-methods-with-trait.md` | varios tipos precisam cumprir o mesmo contrato |
| `virtual-dispatch-with-dyn.md` | valores de tipos diferentes precisam ser chamados pelo mesmo contrato |
| `partial-class-with-apply.md` | voce quer separar metodos sem fragmentar os dados |
| `callable-delegate.md` | voce quer passar uma funcao como valor |
| `zenith-equivalents-from-other-languages.md` | voce conhece o conceito em outra linguagem e quer o equivalente Zenith |
| `intentional-failure-builtins.md` | voce precisa falhar de modo claro com `check`, `todo` ou `unreachable` |

## Validacao

Antes de publicar uma receita:

```powershell
python tools/check_docs_paths.py
git diff --check
```

Exemplos devem compilar ou ser marcados como ilustrativos.

## Historico de atualizacao

- 2026-04-25: indice inicial do cookbook.
- 2026-04-26: adicionada receita de equivalentes Zenith para conceitos de outras linguagens.
- 2026-04-26: adicionada receita de falhas intencionais com `check`, `todo` e `unreachable`.
