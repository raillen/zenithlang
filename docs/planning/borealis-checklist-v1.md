# Borealis Checklist 1.0

## Objetivo

Executar e validar as entregas do roadmap Borealis 1.0 com evidencias objetivas.

## Escopo

- Itens de execucao por milestone do ciclo Borealis.
- Gates obrigatorios de API, exemplos, backend e estabilidade.

## Dependencias

- Upstream:
  - `docs/planning/borealis-roadmap-v1.md`
  - `docs/planning/roadmap-v3.md`
  - `docs/planning/checklist-v3.md`
  - `packages/borealis/architecture-summary.md`
- Downstream:
  - `docs/reports/*`
  - `docs/reports/release/*`
  - `docs/reports/compatibility/*`
- Codigo/Testes relacionados:
  - `./zt.exe check packages/borealis/zenith.ztproj --all`
  - `python build.py` (quando houver mudanca em compiler/runtime)

## Como usar

1. Marque apenas itens com evidencia valida.
2. Execute os gates obrigatorios antes de fechar milestone.
3. Registre risco residual quando houver.

Checklist operacional derivado de `docs/planning/borealis-roadmap-v1.md`.

Status inicial: em andamento (R3.B0 ate R3.B6 concluidos; R3.B7 parcial)
Data de criacao: 2026-04-22

## Gates obrigatorios por milestone

- [x] `./zt.exe check packages/borealis/zenith.ztproj --all` verde
- [x] exemplos do Borealis atualizados quando houver mudanca de API
- [x] decision/checklist atualizados com evidencia minima
- [ ] sem regressao acima do budget da milestone

Quando houver mudanca em compiler/runtime:

- [x] `python build.py` verde

Para release Borealis 1.0:

- [ ] checklist Borealis completo
- [ ] docs de onboarding e limites conhecidos publicados
- [ ] sem P0 aberto sem aceite formal
- [ ] package pronto para fluxo `zpm` (quando comando estiver disponivel)

## R3.B0 - Baseline e alinhamento

- [x] Publicar roadmap e checklist proprios do Borealis
- [x] Publicar tabela-resumo unica da arquitetura Borealis
- [x] Fechar decision de layering (`borealis.game` + `borealis.engine`)
- [x] Revisar e alinhar decision de editor-ready architecture
- [x] Revisar e alinhar decision de stack Borealis
- [x] Registrar `Borealis Flow` como direcao futura do editor
- [x] Registrar baseline da API atual
- [x] Registrar riscos P0/P1/P2 com owner e prazo

Criterio de aceite:

- [x] Base documental e tecnica unica para iniciar o ciclo
- [x] Arquitetura resumida, coerente e pronta para evoluir com editor

## R3.B1 - Naming da camada facil

- [x] Fechar naming canonico da API `borealis.game`
- [x] Definir aliases temporarios (se necessario)
- [x] Cobrir naming final em exemplos pequenos
- [x] Atualizar docs para onboarding rapido

Criterio de aceite:

- [x] Nomes finais claros, consistentes e orientados a facilidade

## R3.B2 - Input v1 (pressed/down/released)

- [x] Implementar `key_pressed`
- [x] Implementar `key_down`
- [x] Implementar `key_released`
- [x] Documentar semantica por frame
- [x] Criar testes de transicao de estado

Criterio de aceite:

- [x] Input previsivel por frame, sem ambiguidade de estado

## R3.B3 - Render2D base

- [x] Implementar `line`
- [x] Implementar `rect` e `rect_outline`
- [x] Implementar `circle` e `circle_outline`
- [x] Implementar `text`
- [x] Criar exemplos com HUD e movimento basico

Criterio de aceite:

- [x] Set minimo de desenho util para prototipos 2D

## R3.B4 - Scene e Entities v1

- [x] Definir API minima de `scene`
- [x] Definir API minima de `entities`
- [x] Implementar create/destroy/find/listagem
- [x] Cobrir tags, hierarquia e snapshot inicial
- [x] Garantir update e draw por frame em cena
- [x] Cobrir fluxo basico com exemplo executavel

Criterio de aceite:

- [x] Organizacao de jogo simples, sem custo cognitivo alto

Evidencia R3.B2-R3.B4:

- [x] `./zt.exe check packages/borealis/zenith.ztproj --all`
- [x] `./zt.exe run tests/behavior/borealis_input_transitions_stub`
- [x] `./zt.exe run tests/behavior/borealis_render2d_stub`
- [ ] `./zt.exe run tests/behavior/borealis_scene_entities_stub` (bloqueado por limite atual do emitter C com tipo `scene.scene`)
- [x] `python build.py`

## R3.B5 - ECS hibrido (interno)

- [x] Definir subset ECS oficial em `borealis.engine`
- [x] Implementar facade simples em `borealis.game`
- [x] Documentar quando migrar da camada facil para engine
- [x] Criar exemplos lado a lado (facil vs avancado)

Criterio de aceite:

- [x] Crescimento tecnico habilitado sem expor complexidade cedo

Evidencia R3.B5 parcial:

- [x] `./zt.exe check packages/borealis/zenith.ztproj --all`
- [x] `./zt.exe run tests/behavior/borealis_ecs_hybrid_stub/zenith.ztproj`

## R3.B6 - Subsistemas modulares (scaffold)

- [x] Criar scaffolds de modulos aprovados em arquitetura
- [x] Cobrir base de `entities`, `movement`, `controllers`, `vehicles`, `animation`, `audio`, `ai`, `camera`, `input`
- [x] Cobrir base de `world`, `procedural`, `ui`, `assets`, `save`, `storage`, `database`, `services`, `settings`, `scene`, `events`, `debug`
- [x] Definir contratos minimos entre subsistemas
- [x] Registrar pontos de extensao para editor futuro
- [x] Criar mapa de dependencias entre modulos

Criterio de aceite:

- [x] Arquitetura modular pronta para expansao incremental

Evidencia R3.B6 parcial:

- [x] `./zt.exe check packages/borealis/zenith.ztproj --all`
- [x] `packages/borealis/examples/modular_scaffolds_v1.zt`
- [x] `packages/borealis/src/borealis/game/contracts.zt`
- [x] `packages/borealis/src/borealis/game/editor.zt`
- [x] `packages/borealis/architecture-summary.md` (Dependency Map B6)

## R3.B7 - Backend desktop inicial

- [x] Implementar adaptador desktop inicial (Raylib/OpenGL)
- [x] Documentar linker profile do backend desktop
- [x] Garantir fallback seguro para stub
- [x] Cobrir E2E de janela + input + draw

Criterio de aceite:

- [ ] Backend real validado sem quebrar caminho de onboarding

Evidencia R3.B7 parcial:

- [x] `packages/borealis/backend-desktop-linker-profile-v1.md`
- [x] `runtime/c/zenith_rt.h` (contrato `zt_borealis_desktop_api`)
- [x] `runtime/c/zenith_rt.c` (adapter Raylib inicial por carga dinamica + fallback `backend_id=1 -> stub`)
- [x] `./zt.exe run tests/behavior/borealis_backend_fallback_stub/zenith.ztproj`

## R3.B8 - Pronto para ZPM

- [ ] Alinhar manifest, docs e exemplos ao fluxo `zpm`
- [ ] Publicar guia de migracao (se houver alias/deprecacao)
- [ ] Definir semver inicial do package
- [ ] Validar instalacao/uso em fluxo limpo

Criterio de aceite:

- [ ] Package preparado para distribuicao

## R3.B9 - Estabilizacao e release Borealis 1.0

- [ ] Fechar changelog do ciclo Borealis
- [ ] Publicar relatorio final de qualidade/compatibilidade
- [ ] Publicar limites conhecidos e risco residual
- [ ] Validar criterios finais de release

Criterio de aceite:

- [ ] Borealis 1.0 publicado com limites e garantias explicitas

## Backlog funcional por modulos (alinhado com as decisions)

Regras deste bloco:

- Cada modulo abaixo deve seguir como fonte canonica as decisions em `packages/borealis/decisions/modules/*.md`.
- N1 = base, N2 = intermediario, N3 = avancado.
- `shape_sweep` e `find_overlaps` ja estao aceitos como naming canonico.
- `move_randomly` e `evade` substituem `wander` e `flee`.

## Entrada principal

- [ ] `game`: manter camada facil clara, curta e amigavel.
- [ ] `engine`: manter camada tecnica separada da jornada iniciante.

## Runtime e jogabilidade

- [ ] `contracts`: contratos comuns para frame/input/body entre modulos.
- [ ] `entities`: create/destroy, tags, hierarquia, snapshot e inspect.
- [ ] `movement`: `move`, `push`, `teleport` e helpers basicos.
- [ ] `controllers`: controladores prontos e ponte opcional com sprite.
- [ ] `vehicles`: familia de veiculos com foco inicial em carro.
- [ ] `animation`: frames, estados visuais, flip e sincronizacao.
- [ ] `audio`: som, musica, volume e grupos.
- [ ] `ai`: `seek`, `evade`, `move_randomly`, patrulha, sensores e FSM/BT.
- [ ] `camera`: follow, bounds, shake, deadzone e framing.
- [ ] `input`: teclado, mouse, gamepad, acoes, contextos e `mouse_hover`.
- [ ] `world`: tiles, tileset, colisao, pathing, `set_solid` e `set_walkable`.
- [ ] `procedural`: seed, noise, pipelines e geracao generica escalavel.

## Interface e experiencia

- [ ] `ui`: widgets, layout, foco, hover, click e fluxo de interface.
- [ ] `ui.hud`: namespace de HUD dentro de `ui`.
- [ ] `scene`: telas, fases, transicoes, pilha e fluxo do jogo.
- [ ] `events`: emit/on/off/queue e mensageria desacoplada.
- [ ] `debug`: grid, hitboxes, watches, FPS e overlays.

## Dados, persistencia e remoto

- [ ] `assets`: load/get/unload/cache de recursos.
- [ ] `save`: slots, autosave, snapshots e restauracao.
- [ ] `storage`: persistencia generica local para texto, JSON e binario.
- [ ] `database`: reservar contrato futuro para SQLite-like backend.
- [ ] `services`: rede, cloud save, APIs, sessoes remotas e multiplayer simples.
- [ ] `settings`: audio, video, idioma, acessibilidade e preferencias.

## Editor futuro

- [ ] `editor-ready`: IDs estaveis, scenes como documento e metadata separada.
- [ ] `editor`: metadata tool-facing (label/note/group/lock/hidden) por stable id.
- [ ] `Borealis Flow`: registrar camada low-code em cima dos modulos existentes.
- [ ] `runtime shared model`: editor futuro deve consumir o mesmo modelo de `entities`, `scene`, `assets`, `components` e snapshots.

## Regras continuas

- [ ] Toda mudanca de API atualiza docs e exemplos no mesmo PR
- [ ] Todo bug novo gera teste de regressao
- [ ] Toda feature nova entra com teste positivo e negativo
- [ ] Toda regressao critica bloqueia fechamento da milestone
- [ ] Toda divergencia entre docs e codigo recebe classificacao P0/P1/P2

## Evidencia minima para marcar item como concluido

- [ ] Comando executado + resultado
- [ ] Arquivo de teste/exemplo novo ou alterado
- [ ] Commit/PR de fechamento
- [ ] Risco residual (se houver)
