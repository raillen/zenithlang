# Borealis

Package de game dev 2D da Zenith com duas camadas publicas:

- `borealis.game`: camada facil (onboarding rapido).
- `borealis.engine`: camada tecnica (controle explicito).

Projeto atual: `Borealis` (nome fechado).

## Estado atual

- `R3.B0` base documental e arquitetural: implementado.
- `R3.B1` naming da camada facil: implementado.
- `R3.B2` input por transicao de frame: implementado.
- `R3.B3` Render2D base: implementado.
- `R3.B4` Scene e Entities v1: implementado.
- `R3.B5` ECS hibrido (subset inicial): implementado.
- `R3.B5` no comportamento atual: fixture ECS validado em `run-pass` (subset de componentes).
- `R3.B6` scaffolds modulares: implementado (com contratos e extensao de editor).
- `R3.B7` backend desktop inicial: parcial.
- `R3.B8` preparo para ZPM: implementado no escopo documental/estrutural.
- `R3.B9` estabilizacao/release: parcial.

## Arquivos de referencia

- arquitetura: `packages/borealis/architecture-summary.md`
- decisions: `packages/borealis/decisions/*`
- baseline da API: `packages/borealis/api-baseline-v1.md`
- linker profile desktop: `packages/borealis/backend-desktop-linker-profile-v1.md`
- changelog: `packages/borealis/CHANGELOG.md`
- limites conhecidos: `packages/borealis/known-limits-v1.md`
- guia de migracao: `packages/borealis/migration-guide-v1.md`
- preparo para ZPM: `packages/borealis/zpm-prep-v1.md`
- riscos ativos: `packages/borealis/risks.md`
- roadmap: `docs/planning/borealis-roadmap-v1.md`
- checklist: `docs/planning/borealis-checklist-v1.md`

## Exemplo rapido (camada facil)

Arquivo: `packages/borealis/examples/minimal_loop.zt`

Fluxo principal:

1. `game.start(config)`
2. `while game.running(ctx)`
3. `game.frame_begin(ctx)`
4. desenhar (`game.draw_rect`, `game.draw_text`)
5. `game.frame_end(ctx)`
6. `game.close(ctx)`

Outros exemplos:

- `packages/borealis/examples/render2d_base.zt` (line/rect/circle/text + HUD simples)
- `packages/borealis/examples/scene_entities_v1.zt` (scene + entities + tags/hierarquia)
- `packages/borealis/examples/ecs_hybrid_v1.zt` (components + systems via `engine.ecs` e facade em `game.entities`)
- `packages/borealis/examples/modular_scaffolds_v1.zt` (uso basico dos novos modulos scaffold)
- `packages/borealis/examples/raylib_desktop_loop.zt` (camada `borealis.game` com `Backend.Raylib`)
- `packages/borealis/examples/raylib_desktop_app/` (projeto completo com `zenith.ztproj`, usando ABI runtime direta para rodar agora)

## Nota sobre backend

Nesta fase, o caminho padrao continua sendo o backend stub para bootstrap.

No R3.B7, o runtime ganhou hook de adapter desktop (`zt_borealis_desktop_api`) e adapter Raylib inicial com carregamento dinamico.

Quando Raylib nao estiver disponivel no ambiente, o fallback para stub continua seguro.

No workspace local, `./zt.exe` agora prioriza runtime/stdlib ao lado do executavel antes de usar `ZENITH_HOME`, evitando misturar a instalacao global com o codigo que esta sendo editado.

Detalhes do profile em `packages/borealis/backend-desktop-linker-profile-v1.md`.

## Rodar exemplo Raylib

1. Garanta que a biblioteca do Raylib esteja disponivel no ambiente:
   - Windows: `raylib.dll` no mesmo diretorio do executavel ou no `PATH`.
   - Linux: `libraylib.so` disponivel no loader path (`LD_LIBRARY_PATH` ou install do sistema).
2. Use `packages/borealis/examples/raylib_desktop_loop.zt` como template do `app.main` do seu projeto Zenith.
3. No seu `main`, mantenha `backend: game.Backend.Raylib`.
4. Rode o projeto normalmente:
   - `./zt.exe run caminho/do/seu/zenith.ztproj`

Projeto pronto incluido no repo:

- `./zt.exe run packages/borealis/examples/raylib_desktop_app/zenith.ztproj`

Se Raylib nao for encontrado, o runtime entra em fallback seguro para `stub`.

## Semver e package

Manifesto atual:

- `name = "borealis"`
- `version = "0.1.0"`
- `kind = "lib"`
- `root_namespace = "borealis"`

Fluxo atual:

- o package ja esta preparado para ZPM
- o comando `zpm` ainda nao esta disponivel nesta fase
- a referencia de preparo esta em `packages/borealis/zpm-prep-v1.md`
