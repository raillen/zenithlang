# Borealis

Package de game dev da Zenith com base 2D e primeiro slice 3D experimental.
Ele mantem duas camadas publicas:

- `borealis.game`: camada facil (onboarding rapido).
- `borealis.engine`: camada tecnica (controle explicito).

Projeto atual: `Borealis` (nome fechado).

## Estado atual

- `R3.B0` base documental e arquitetural: implementado.
- `R3.B1` naming da camada facil: implementado.
- `R3.B2` input por transicao de frame: implementado.
- `R3.B3` Render2D base: implementado.
- `R3.B4` Scene e Entities v1: implementado.
- `R3.B4` no comportamento atual: fixture Scene/Entities validado em `run-pass`.
- `R3.B5` ECS hibrido (subset inicial): implementado.
- `R3.B5` no comportamento atual: fixture ECS validado em `run-pass` (subset de componentes).
- `R3.B6` scaffolds modulares: implementado (com contratos e extensao de editor).
- `R3.B6` no comportamento atual: fixture foundations validando assets, events com listeners/queue, debug, UI/HUD, editor metadata, save, storage, services, database e settings persistente.
- `R3.B7` backend desktop inicial: parcial, com fallback seguro e smoke do binding Raylib validados.
- `R3.B7.3D` slice 3D experimental: `BeginMode3D/EndMode3D`, grid, cubo, OBJ model, billboard e smoke Raylib real.
- `R3.B8` preparo para ZPM: implementado no escopo documental/estrutural.
- `R3.B9` estabilizacao/release: parcial.

## Arquivos de referencia

- arquitetura: `packages/borealis/architecture-summary.md`
- decisions: `packages/borealis/decisions/*`
- baseline da API: `packages/borealis/api-baseline-v1.md`
- linker profile desktop: `packages/borealis/backend-desktop-linker-profile-v1.md`
- changelog: `packages/borealis/CHANGELOG.md`
- limites conhecidos: `packages/borealis/known-limits-v1.md`
- gaps futuros da linguagem: `packages/borealis/language-gaps-v1.md`
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

## Scene document inicial

O primeiro contrato JSON de cena esta em `packages/borealis/scenes/sample.scene.json`.

Ele e pequeno de proposito: `name`, `document_id` e uma lista de `entities` com
`stable_id`, `name`, `layer`, `parent`, `tags`, `transform` e `components`.
Esse arquivo serve como fixture compartilhada entre Borealis e `tools/borealis-editor`.
O preview runner do editor ja usa esse contrato em `open_scene` e emite
diagnostico com a contagem de entidades carregadas.

## Nota sobre backend

Nesta fase, o caminho padrao continua sendo o backend stub para bootstrap.

No R3.B7, o runtime ganhou hook de adapter desktop (`zt_borealis_desktop_api`) e adapter Raylib inicial com carregamento dinamico.

Quando Raylib nao estiver disponivel no ambiente, o fallback para stub continua seguro.

O runtime tambem procura binarios nativos dentro do modulo Borealis. O caminho
recomendado e `packages/borealis/native/raylib/<plataforma>/`, por exemplo
`packages/borealis/native/raylib/windows-x64/raylib.dll`.

No workspace local, `./zt.exe` agora prioriza runtime/stdlib ao lado do executavel antes de usar `ZENITH_HOME`, evitando misturar a instalacao global com o codigo que esta sendo editado.

O backend C desta fase tambem ja cobre o caminho de `optional<Struct>` com tipos qualificados e payload gerenciado, o que permitiu restaurar filas de eventos tipadas em `borealis.game.events`.

No estado atual da API, `save`, `storage` e cloud stub em `services` ja preservam string vazia como valor valido. Em `save`, isso cobre tanto snapshot de slot quanto campos por slot (`slot_field_*`). Remocao continua retornando `none`.

No estado atual da API, `assets` ja cobre loaders especificos como `image`, `texture`, `font`, `sound`, `music`, `shader`, `atlas` e `spritesheet`, preserva `id` estavel e separa asset registrado de asset carregado.

No estado atual da API, `settings` ja cobre audio, idioma, acessibilidade, fullscreen, `vsync`, `render_scale`, `ui_scale` e perfis simples persistidos em `storage`.

No estado atual da API, `events` ja cobre fila global, listeners nomeados, `once`, `listener_peek/poll` e limpeza seletiva por nome ou listener.

No estado atual da API, `debug` ja cobre logs, flags, watches e overlays textuais simples para grid, bounds, hitbox, FPS e resumo agregado.

No estado atual da API, `ui` ja cobre widgets simples, foco, hover/click, visibilidade e layout linear. `ui.hud` cobre widgets de HUD com texto, valor, posicao e visibilidade.

No estado atual da API, `scene` ja cobre fase, `document_id` e pilha simples de fluxo. `editor` guarda metadata separada por stable id.

Detalhes do profile em `packages/borealis/backend-desktop-linker-profile-v1.md`.

## Rodar exemplo Raylib

1. Garanta que a biblioteca do Raylib esteja disponivel no ambiente:
   - Windows: `raylib.dll` no mesmo diretorio do executavel ou no `PATH`.
   - Linux: `libraylib.so` disponivel no loader path (`LD_LIBRARY_PATH` ou install do sistema).
2. Ou coloque o binario dentro do modulo:
   - Windows x64: `packages/borealis/native/raylib/windows-x64/raylib.dll`.
   - Windows x64 (layout oficial do release): `packages/borealis/native/raylib/windows-x64/lib/raylib.dll`.
   - Linux x64: `packages/borealis/native/raylib/linux-x64/libraylib.so`.
   - macOS arm64: `packages/borealis/native/raylib/macos-arm64/libraylib.dylib`.
3. Se quiser apontar para outro local, use `BOREALIS_RAYLIB_PATH`.
4. Use `packages/borealis/examples/raylib_desktop_loop.zt` como template do `app.main` do seu projeto Zenith.
5. No seu `main`, mantenha `backend: game.Backend.Raylib`.
6. Rode o projeto normalmente:
   - `./zt.exe run caminho/do/seu/zenith.ztproj`

Para diagnostico, a API expoe:

- `borealis.raylib_available()`
- `borealis.raylib_loaded_path()`
- `borealis.game.raylib_available()`
- `borealis.game.raylib_loaded_path()`
- `borealis.raylib.available()`
- `borealis.raylib.loaded_path()`
- `borealis.raylib.require_available()`

Para usar nomes mais proximos da Raylib, importe `borealis.raylib`. Esse
modulo ja cobre a base de janela/frame, shapes, texto, input, `measure_text`,
handles simples de textura/som/modelo, helpers de `raymath`/`reasings` e o
primeiro caminho 3D Raylib (`begin_mode3d`, `draw_cube`, `draw_grid`,
`load_model`, `draw_model` e `draw_billboard`).

Para fluxos que realmente dependem da DLL nativa, como carregar textura, som ou
inicializar audio, use `borealis.raylib.require_available()?` antes do passo
real de asset/audio. O modulo tambem retorna erro claro para caminho vazio em
`load_texture("")` e `load_sound("")`.

Smoke test incluido no repo:

- `./zt.exe run tests/behavior/borealis_raylib_binding_stub/zenith.ztproj`
- `./zt.exe run tests/behavior/borealis_raylib_assets_real/zenith.ztproj`
- `./zt.exe run tests/behavior/borealis_render3d_basic/zenith.ztproj`

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
