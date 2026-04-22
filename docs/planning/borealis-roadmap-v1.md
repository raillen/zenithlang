# Borealis Roadmap 1.0

## Objetivo

Definir o ciclo inicial da game lib Borealis como package modular, com experiencia facil para iniciantes e caminho tecnico para projetos maiores.

Foco do ciclo:

- consolidar `borealis.game` como camada de entrada;
- consolidar `borealis.engine` como camada tecnica;
- preparar subsistemas modulares sem quebrar simplicidade inicial.

## Escopo

- Planejamento do ciclo Borealis (R3.B0 ate R3.B9).
- Entregas orientadas a package (`packages/borealis`), nao a engine/editor completo.
- Regras explicitas do que entra e do que nao entra.

## Dependencias

- Upstream:
  - `docs/planning/roadmap-v3.md`
  - `docs/planning/checklist-v3.md`
  - `packages/borealis/*`
  - `packages/borealis/decisions/*`
- Downstream:
  - `docs/planning/borealis-checklist-v1.md`
  - `packages/borealis/architecture-summary.md`
  - `docs/reports/release/*`
  - `docs/reports/compatibility/*`

Status: em andamento (R3.B0 ate R3.B6 concluidos; R3.B7 parcial)
Data: 2026-04-22
Base: Borealis package + stub runtime + linker hook

Progresso atual:

- R3.B0 concluido.
- R3.B1 concluido.
- R3.B2 concluido.
- R3.B3 concluido.
- R3.B4 concluido (API/scaffold); fixture `run-pass` de scene tipada ainda bloqueada no emitter C.
- R3.B5 concluido (subset ECS interno + facade inicial de componentes).
- R3.B6 concluido (scaffolds + contratos + extensao editor + mapa de dependencias).
- R3.B7 em andamento (adapter Raylib inicial + fallback + E2E fallback).
- Proxima etapa ativa: R3.B7 (validacao em ambiente com Raylib instalado).

## Diretrizes do ciclo Borealis

1. API facil primeiro: uso padrao por `borealis.game`.
2. Escape hatch claro: `borealis.engine` para controle avancado.
3. Backend e linker nao devem ser requisito de onboarding.
4. Semantica de frame explicita (`frame_begin`/`frame_end`).
5. Evolucao por adicao, evitando quebra desnecessaria de API.
6. Borealis segue como package (nao entra em `stdlib` neste ciclo).

## Gates obrigatorios por milestone

Nenhuma milestone avanca sem:

1. `./zt.exe check packages/borealis/zenith.ztproj --all` verde
2. `python build.py` verde quando houver mudanca em compiler/runtime
3. exemplos do Borealis atualizados quando houver mudanca de API
4. decision/checklist atualizados com evidencia minima

Para release do ciclo Borealis:

1. checklist Borealis completo
2. docs de onboarding e limites conhecidos publicados
3. sem P0 aberto sem aceite formal
4. pacote pronto para fluxo `zpm` quando comando estiver disponivel

## Fases do Roadmap Borealis 1.0

## R3.B0 - Baseline e alinhamento

Objetivo:

- consolidar base tecnica e documental da lib.

Entregas:

- roadmap e checklist proprios do Borealis;
- tabela-resumo unica da arquitetura Borealis publicada;
- decision de layering (`game` + `engine`) aceita;
- decision de editor-ready architecture revisada e alinhada;
- decision de stack Borealis revisada e alinhada;
- decision de `Borealis Flow` registrada como direcao futura do editor;
- baseline de API atual e riscos P0/P1/P2 documentados.

## R3.B1 - Naming da camada facil

Objetivo:

- fechar nomes canonicos da camada `borealis.game`.

Entregas:

- decision de naming (start/close/running/frame_begin/frame_end/input);
- aliases temporarios definidos quando necessario;
- exemplos pequenos cobrindo o naming final.

## R3.B2 - Input v1 (pressed/down/released)

Objetivo:

- garantir modelo de input simples e previsivel por frame.

Entregas:

- `key_pressed`, `key_down`, `key_released`;
- semantica formal por frame;
- testes de transicao de estado.

Status da fase: concluida no escopo de API/scaffold; fixture `run-pass` tipada segue bloqueada no emitter C.

## R3.B3 - Render2D base

Objetivo:

- entregar desenho util para prototipos 2D.

Entregas:

- `line`, `rect`, `rect_outline`, `circle`, `circle_outline`, `text`;
- helpers de cor/area consistentes;
- exemplos com HUD e movimento basico.

Status da fase: concluida.

## R3.B4 - Scene e Entities v1

Objetivo:

- introduzir organizacao de codigo sem complexidade excessiva.

Entregas:

- API minima de `scene` e `entities`;
- create/destroy/find/listagem de entidades;
- tags, hierarquia e snapshot inicial de entidades;
- desenho por cena e atualizacao por frame.

Status da fase: concluida.

## R3.B5 - ECS hibrido (interno)

Objetivo:

- preparar crescimento tecnico sem expor complexidade cedo.

Entregas:

- componentes e sistemas no `engine` (subset oficial);
- facade simples no `game` para uso padrao;
- guidelines de quando sair da camada facil.

Status parcial da execucao:

- fixture `tests/behavior/borealis_ecs_hybrid_stub` validado em `run-pass` (subset de componentes);
- validacao `run-pass` do caminho de sistemas tipados com `struct` permanece no backlog do backend C.

## R3.B6 - Subsistemas modulares (scaffold)

Objetivo:

- preparar arquitetura para expansao e editor futuro.

Entregas:

- scaffolds de modulos aprovados em arquitetura;
- base de `entities`, `movement`, `controllers`, `vehicles`, `animation`, `audio`, `ai`, `camera`, `input`;
- base de `world`, `procedural`, `ui`, `assets`, `save`, `storage`, `database`, `services`, `settings`, `scene`, `events`, `debug`;
- contratos minimos de integracao entre subsistemas;
- metadados iniciais para futura integracao com editor.

Status parcial da execucao:

- scaffolds base criados para `movement`, `controllers`, `vehicles`, `animation`, `audio`, `ai`, `camera`, `input`;
- scaffolds base criados para `world`, `procedural`, `ui`, `ui.hud`, `assets`, `save`, `storage`, `database`, `services`, `settings`, `events`, `debug`;
- exemplo de integracao inicial publicado em `packages/borealis/examples/modular_scaffolds_v1.zt`.

Status da fase: concluida.

## R3.B7 - Backend desktop inicial

Objetivo:

- validar backend real apos stub.

Entregas:

- adaptador desktop inicial (Raylib/OpenGL) com linker profile documentado;
- fallback seguro para stub quando backend real nao estiver ligado;
- teste E2E de janela + input + draw.

Status parcial da execucao:

- hook de adapter desktop adicionado no runtime (`zt_borealis_desktop_api`, `zt_borealis_set_desktop_api`, `zt_borealis_get_desktop_api`);
- adapter Raylib inicial implementado no runtime por carga dinamica de biblioteca;
- fallback seguro para stub quando `backend_id=1` e adapter nao esta disponivel no ambiente;
- profile de linker desktop publicado em `packages/borealis/backend-desktop-linker-profile-v1.md`;
- E2E de janela + input + draw publicado em `tests/behavior/borealis_backend_fallback_stub`.

## R3.B8 - Pronto para ZPM

Objetivo:

- deixar package pronto para distribuicao.

Entregas:

- manifest, docs e exemplos alinhados com fluxo `zpm`;
- guia de migracao de versao (se houver alias/deprecacao);
- semver inicial da lib definido.

## R3.B9 - Estabilizacao e release Borealis 1.0

Objetivo:

- fechar ciclo com pacote utilizavel e limites claros.

Entregas:

- changelog do ciclo Borealis;
- relatorio final de qualidade/compatibilidade;
- limites conhecidos e risco residual publicados.

## Catalogo funcional por modulos (alinhado com as decisions)

Este catalogo substitui o backlog antigo por topicos grandes.

Agora o Borealis esta organizado por modulos, porque foi assim que as decisions ficaram fechadas.

Niveis usados no ciclo:

- N1 = base simples, boa para onboarding e prototipacao
- N2 = camada intermediaria, util para jogo real
- N3 = camada avancada, ja pensando em editor, escala e automacao

Fonte canonica:

- `packages/borealis/architecture-summary.md`
- `packages/borealis/decisions/modules/*.md`
- `packages/borealis/decisions/005-editor-ready-architecture.md`
- `packages/borealis/decisions/006-borealis-stack.md`
- `packages/borealis/decisions/007-borealis-flow.md`

## Regras canonicas de naming ja fechadas

- `shape_sweep` e `find_overlaps` deixam de ser nomes em aberto e passam a ser nomes aceitos.
- `wander` e `flee` deixam de ser nomes canonicos; usar `move_randomly` e `evade`.
- `seek` permanece como nome aceito.
- `Vector2`, `Point2`, `Size2`, `Rect` e `Direction` seguem como base de tipos da camada facil.

## Mapa funcional por modulo

### Entrada principal

- `game`: camada facil para quem esta comecando.
- `engine`: camada tecnica para controle avancado.

### Runtime e jogabilidade

- `contracts`: contratos comuns de frame/input/body para integracao entre modulos.
- `entities`: ponte amigavel para ECS, com create/destroy, tags, hierarquia, snapshot e inspect.
- `movement`: movimento generico como `move`, `push`, `teleport` e helpers.
- `controllers`: controladores prontos como platform, topdown, 4 directions, 8 directions, grid, twin stick e ponte opcional com sprite.
- `vehicles`: controladores especializados de veiculos, com foco inicial em carro e espaco para familias futuras.
- `animation`: animacoes, estados visuais e sincronizacao de sprite.
- `audio`: som, musica, grupos e controle de volume.
- `ai`: comportamento, percepcao, busca de alvo, `seek`, `evade`, `move_randomly`, patrulha e FSM/BT.
- `camera`: follow, bounds, shake, deadzone e enquadramento.
- `input`: teclado, mouse, gamepad, acoes, contextos e `mouse_hover`.
- `world`: tiles, tileset, colisao, pathing e estrutura do mundo.
- `procedural`: geracao generica escalavel, incluindo mapas, layouts, noise, seed e pipelines.

### Interface e experiencia

- `ui`: widgets, layout, interacao e fluxo de interface.
- `ui.hud`: namespace de HUD dentro de `ui`.
- `scene`: organizacao de telas, fases, transicoes e pilha de cenas.
- `events`: mensageria desacoplada e leve.
- `debug`: overlays, hitboxes, watches, FPS e inspecao visual.

### Dados, persistencia e remoto

- `assets`: load/get/unload/cache de texturas, fontes, som, atlas e sprite sheets.
- `save`: progresso, slots, autosave, snapshots e restauracao de estado.
- `storage`: persistencia generica local para texto, JSON e binario.
- `database`: reservado para backend futuro estilo SQLite.
- `services`: modulo guarda-chuva para rede, cloud save, APIs, sessoes remotas e multiplayer simples.
- `settings`: configuracoes de usuario, acessibilidade, audio, video e idioma.

### Editor futuro

- `Borealis Flow`: camada visual low-code do editor futuro, em cima dos modulos do Borealis, sem substituir Zenith.
- `editor`: metadados de ferramenta (label/note/group/lock/hidden) ligados por stable id.
- editor futuro deve consumir o mesmo modelo de runtime para `entities`, `scene`, `assets`, `components` e snapshots.

## Fora de escopo do Borealis 1.0

- editor visual completo
- tilemap editor completo
- pipeline de importacao de assets avancado
- 3D
- netcode multiplayer completo
- banco de dados completo na runtime inicial
- no-code completo cobrindo toda a engine

## Definicao de pronto por milestone

Uma milestone so fecha quando tiver:

1. API/decision atualizada (quando houver mudanca de comportamento);
2. exemplos e testes minimos da milestone;
3. checklist Borealis com evidencia;
4. sem regressao acima de budget definido para a milestone;
5. docs de uso e limites conhecidos atualizados.
