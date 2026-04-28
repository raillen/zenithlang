# Learn Zenith

> Audience: user
> Status: draft
> Surface: public
> Source of truth: no

## Objetivo

Ensinar Zenith em ordem, com baixa friccao.

Esta trilha e para quem quer aprender a linguagem sem ler specs internas,
decisions ou relatórios de implementação.

## Escopo

Esta pasta deve conter guias didaticos.

Cada guia deve:

- explicar uma ideia por vez;
- usar exemplos pequenos;
- mostrar erros comuns;
- dizer os limites atuais da feature;
- apontar para a referência quando a pessoa quiser detalhes.

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

Ordem inicial recomendada:

0. `learn-zenith-in-30-min.md`: tour rapido em ingles.
1. `01-first-program.md`: primeiro arquivo Zenith.
2. `02-values-and-functions.md`: valores, tipos e funções.
3. `03-errors-and-absence.md`: `optional`, `result` e `panic`.
4. Structs, traits, apply e enums. Ainda pendente.
5. Generics, callables, `dyn` e recursos avançados. Ainda pendente.
6. Testes, formatter e diagnostics. Ainda pendente.

## Validação

Antes de publicar uma página desta trilha:

```powershell
python tools/check_docs_paths.py
git diff --check
```

Exemplos devem vir de behavior tests quando possível.

## Histórico de atualização

- 2026-04-25: índice inicial da trilha de aprendizado.
- 2026-04-28: added English 30-minute tutorial.
