# Language Documentation Feature Matrix

> Audience: maintainer
> Status: draft
> Surface: reference
> Source of truth: no

## Objetivo

Rastrear cobertura documental das features da linguagem.

Esta matriz mostra o que esta bem documentado, parcial ou ausente.

## Escopo

Inclui features da linguagem, stdlib essencial e tooling que aparecem para o
usuario.

Nao substitui:

- `language/spec/implementation-status.md`;
- `tests/behavior/MATRIX.md`;
- `language/decisions/README.md`.

## Dependencias

- Upstream:
  - `tests/behavior/MATRIX.md`
  - `language/spec/implementation-status.md`
  - `docs/internal/planning/documentation-roadmap-v1.md`
- Downstream:
  - `docs/public/learn/`
  - `docs/public/cookbook/`
  - `docs/reference/language/`
- Codigo/Testes relacionados:
  - `tools/check_docs_paths.py`

## Conteudo principal

Status permitido:

- `complete`: guia, referencia, exemplo e teste existem.
- `partial`: existe documentacao, mas falta detalhe, exemplo ou limite.
- `missing`: feature existe, mas nao tem documentacao suficiente.
- `planned`: decisao/spec existe, mas a feature nao esta pronta para usuario.
- `rejected`: documentado como nao-canonico ou fora da filosofia atual.

| Feature | Guia | Referencia | Cookbook | Spec | Teste | Status |
| --- | --- | --- | --- | --- | --- | --- |
| Forma de arquivo | partial | partial | missing | complete | complete | partial |
| Namespace/import | partial | partial | missing | complete | complete | partial |
| `const`/`var` | partial | partial | missing | complete | complete | partial |
| `optional<T>` | missing | partial | partial | complete | complete | partial |
| `result<T,E>` | missing | partial | partial | complete | complete | partial |
| `enum` com payload | missing | missing | partial | complete | complete | partial |
| `trait`/`apply` | missing | partial | partial | complete | complete | partial |
| `dyn<Trait>` | missing | partial | partial | complete | complete | partial |
| callables/delegates | missing | missing | partial | complete | complete | partial |
| closures/lambdas | missing | missing | missing | complete | complete | missing |
| `lazy<T>` | missing | partial | missing | complete | complete | partial |
| `where` contracts | missing | partial | missing | complete | complete | partial |
| formatter | missing | missing | missing | complete | complete | missing |
| diagnostics | missing | partial | missing | complete | complete | partial |

## Validacao

Atualize esta matriz quando:

- uma feature nova entrar;
- uma pagina publica for criada;
- uma referencia for expandida;
- uma feature mudar de status.

## Historico de atualizacao

- 2026-04-25: matriz inicial criada com cobertura aproximada.
