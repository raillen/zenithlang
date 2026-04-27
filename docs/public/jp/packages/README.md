# Package Guides

> Guias publicos para packages oficiais Zenith.
> Audience: user, package-author
> Surface: public
> Status: current

## Objetivo

Explicar como usar packages oficiais quando eles estiverem prontos para consumo publico.

## Packages atuais

| Package | Estado | Observacao |
| --- | --- | --- |
| `borealis` | experimental | framework/game package em evolução |

## Regra para documentar package publico

Antes de promover um package para guia publico, confirme:

1. API principal esta estável para o ciclo atual.
2. Exemplo mínimo passa em `zt check`.
3. Limites conhecidos estao documentados.
4. Roadmap interno não aparece como promessa publica.

## Borealis

A documentacao técnica do Borealis fica junto do package:

- Guia publico experimental: `docs/public/packages/borealis.md`.
- `packages/borealis/README.md`.
- `packages/borealis/decisions/`.
- `docs/internal/planning/borealis-roadmap-v1.md`.

Quando a API estiver pronta para usuários externos, este guia deve receber um caminho curto de uso.
