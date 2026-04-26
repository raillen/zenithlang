# Package Borealis

> Audience: user, package-author
> Status: experimental
> Surface: public
> Source of truth: no

## Resumo

Borealis e o package experimental de engine/framework dentro deste repositorio.

Ele ainda esta evoluindo.

Use esta pagina como mapa publico, nao como contrato final de API.

## Quando olhar para Borealis

Use Borealis quando quiser acompanhar:

- APIs de jogo;
- entidades;
- eventos;
- assets;
- UI/HUD;
- runtime 2D/3D em evolucao;
- integracao futura com Studio.

## Onde esta o package

```text
packages/borealis/
  zenith.ztproj
  src/borealis/
```

## Como checar

Na raiz do repositorio:

```powershell
.\zt.exe check packages\borealis\zenith.ztproj --all
```

## Exemplo de uso

O uso publico ainda deve ser guiado pelos fixtures de comportamento.

Comece por:

- `tests/behavior/borealis_foundations_stub/`
- `tests/behavior/borealis_runtime_gameplay_stub/`

## Limites atuais

- API ainda experimental.
- Nem todo recurso planejado tem reflexo completo em runtime real.
- Parte da integracao com Studio ainda pertence a roadmaps internos.

## Fontes tecnicas

- `packages/borealis/README.md`
- `packages/borealis/api-baseline-v1.md`
- `docs/internal/planning/borealis-roadmap-v1.md`
- `docs/internal/planning/borealis-engine-roadmap-v2.md`
