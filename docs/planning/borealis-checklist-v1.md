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

Status inicial: nao iniciado
Data de criacao: 2026-04-22

## Gates obrigatorios por milestone

- [ ] `./zt.exe check packages/borealis/zenith.ztproj --all` verde
- [ ] exemplos do Borealis atualizados quando houver mudanca de API
- [ ] decision/checklist atualizados com evidencia minima
- [ ] sem regressao acima do budget da milestone

Quando houver mudanca em compiler/runtime:

- [ ] `python build.py` verde

Para release Borealis 1.0:

- [ ] checklist Borealis completo
- [ ] docs de onboarding e limites conhecidos publicados
- [ ] sem P0 aberto sem aceite formal
- [ ] package pronto para fluxo `zpm` (quando comando estiver disponivel)

## R3.B0 - Baseline e alinhamento

- [ ] Publicar roadmap e checklist proprios do Borealis
- [ ] Publicar tabela-resumo unica da arquitetura Borealis
- [ ] Fechar decision de layering (`borealis.game` + `borealis.engine`)
- [ ] Revisar e alinhar decision de editor-ready architecture
- [ ] Registrar baseline da API atual
- [ ] Registrar riscos P0/P1/P2 com owner e prazo

Criterio de aceite:

- [ ] Base documental e tecnica unica para iniciar o ciclo
- [ ] Arquitetura resumida, coerente e pronta para evoluir com editor

## R3.B1 - Naming da camada facil

- [ ] Fechar naming canonico da API `borealis.game`
- [ ] Definir aliases temporarios (se necessario)
- [ ] Cobrir naming final em exemplos pequenos
- [ ] Atualizar docs para onboarding rapido

Criterio de aceite:

- [ ] Nomes finais claros, consistentes e orientados a facilidade

## R3.B2 - Input v1 (pressed/down/released)

- [ ] Implementar `key_pressed`
- [ ] Implementar `key_down`
- [ ] Implementar `key_released`
- [ ] Documentar semantica por frame
- [ ] Criar testes de transicao de estado

Criterio de aceite:

- [ ] Input previsivel por frame, sem ambiguidade de estado

## R3.B3 - Render2D base

- [ ] Implementar `line`
- [ ] Implementar `rect` e `rect_outline`
- [ ] Implementar `circle` e `circle_outline`
- [ ] Implementar `text`
- [ ] Criar exemplos com HUD e movimento basico

Criterio de aceite:

- [ ] Set minimo de desenho util para prototipos 2D

## R3.B4 - Scene e Entity v1

- [ ] Definir API minima de `scene`
- [ ] Definir API minima de `entity`
- [ ] Implementar spawn/remove/listagem
- [ ] Garantir update e draw por frame em cena
- [ ] Cobrir fluxo basico com exemplo executavel

Criterio de aceite:

- [ ] Organizacao de jogo simples, sem custo cognitivo alto

## R3.B5 - ECS hibrido (interno)

- [ ] Definir subset ECS oficial em `borealis.engine`
- [ ] Implementar facade simples em `borealis.game`
- [ ] Documentar quando migrar da camada facil para engine
- [ ] Criar exemplos lado a lado (facil vs avancado)

Criterio de aceite:

- [ ] Crescimento tecnico habilitado sem expor complexidade cedo

## R3.B6 - Subsistemas modulares (scaffold)

- [ ] Criar scaffolds de `physics2d`, `audio`, `controls`, `assets`, `time`
- [ ] Definir contratos minimos entre subsistemas
- [ ] Registrar pontos de extensao para editor futuro
- [ ] Criar mapa de dependencias entre modulos

Criterio de aceite:

- [ ] Arquitetura modular pronta para expansao incremental

## R3.B7 - Backend desktop inicial

- [ ] Implementar adaptador desktop inicial (Raylib/OpenGL)
- [ ] Documentar linker profile do backend desktop
- [ ] Garantir fallback seguro para stub
- [ ] Cobrir E2E de janela + input + draw

Criterio de aceite:

- [ ] Backend real validado sem quebrar caminho de onboarding

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

## Backlog funcional por nivel (detalhado)

Regras deste bloco:

- Cada funcao tem descricao curta.
- N1 = base, N2 = intermediario, N3 = avancado.
- Nomes `shape_sweep` e `find_overlaps` estao em validacao (sem decisao final).

## Colisao

N1:

- [ ] `collides(a, b) -> bool`: testa se dois shapes se sobrepoem.
- [ ] `contains_point(shape, point) -> bool`: verifica se ponto esta dentro do shape.
- [ ] `overlap_query(area) -> Entity[]`: retorna entidades dentro de uma area.
- [ ] `raycast(origin, direction, max_distance) -> Hit`: retorna primeiro impacto no raio.
- [ ] `linecast(start, end) -> Hit`: raycast simplificado por dois pontos.
- [ ] `shape_sweep(shape, motion) -> Hit`: detecta colisao ao deslocar shape.

N2:

- [ ] `find_nearest(shape, radius) -> Entity`: busca entidade mais proxima.
- [ ] `find_overlaps(shape, filter) -> Entity[]`: busca sobreposicoes com filtro.
- [ ] `collision_normal(hit) -> Vector2`: retorna normal da colisao.
- [ ] `separation_vector(a, b) -> Vector2`: menor vetor para separar corpos.

N3:

- [ ] `move_and_collide(entity, motion) -> CollisionInfo`: move com retorno de colisao.
- [ ] `move_and_slide(entity, velocity, up) -> MotionResult`: move deslizando em superficies.
- [ ] `shape_cast(shape, direction, distance) -> Hit`: cast volumetrico do shape.
- [ ] `predict_collision(entity, motion) -> Hit`: preve colisao antes do movimento real.
- [ ] `get_contacts(entity) -> Contact[]`: lista contatos atuais da entidade.

## Fisica

N1:

- [ ] `create_body(body_type) -> Body`: cria corpo fisico.
- [ ] `set_velocity(body, velocity)`: define velocidade linear.
- [ ] `add_force(body, force)`: aplica forca continua.
- [ ] `add_impulse(body, impulse)`: aplica impulso instantaneo.
- [ ] `set_gravity_scale(body, scale)`: ajusta escala de gravidade.
- [ ] `set_mass(body, mass)`: define massa.
- [ ] `physics_step(dt)`: avanca simulacao fisica.

N2:

- [ ] `set_drag(body, drag)`: define resistencia ao movimento.
- [ ] `set_friction(body, friction)`: define atrito.
- [ ] `set_bounciness(body, value)`: define quique/restituicao.
- [ ] `lock_rotation(body, locked)`: trava rotacao.

N3:

- [ ] `character_body(config) -> Character`: cria corpo de personagem.
- [ ] `move_character(character, input, dt) -> MotionResult`: move personagem com colisao.
- [ ] `is_on_floor(character) -> bool`: verifica contato com chao.
- [ ] `is_on_wall(character) -> bool`: verifica contato lateral.
- [ ] `apply_knockback(body, direction, force)`: aplica recuo de gameplay.
- [ ] `rewind_physics(state)`: restaura estado anterior da simulacao.

## Sprites

N1:

- [ ] `sprite_draw(texture, position)`: desenha textura.
- [ ] `sprite_draw_ex(texture, transform)`: desenha com transformacao.
- [ ] `sprite_draw_region(texture, source, dest)`: desenha regiao da textura.
- [ ] `sprite_set_color(sprite, color)`: aplica cor no sprite.
- [ ] `sprite_set_flip(sprite, flip_x, flip_y)`: espelha sprite.

N2:

- [ ] `anim_create(frames, fps, loop) -> Animation`: cria animacao por frames.
- [ ] `anim_play(sprite, animation)`: inicia animacao.
- [ ] `anim_update(sprite, dt)`: atualiza frame da animacao.
- [ ] `anim_set_speed(sprite, speed)`: ajusta velocidade da animacao.

N3:

- [ ] `animator_create() -> Animator`: cria maquina de estados de animacao.
- [ ] `animator_set_state(animator, state)`: troca estado do animator.
- [ ] `anim_sync_with_velocity(sprite, velocity)`: sincroniza animacao com velocidade.
- [ ] `anim_directional(sprite, direction)`: seleciona animacao por direcao.
- [ ] `sprite_batch_create() -> Batch`: cria sistema de render em lote.

## Shaders e FX

N1:

- [ ] `shader_create(vs, fs) -> Shader`: cria shader.
- [ ] `shader_set_uniform(shader, name, value)`: define uniform.
- [ ] `shader_apply(target, shader)`: aplica shader no alvo.

N2:

- [ ] `material_create(shader) -> Material`: cria material a partir de shader.
- [ ] `material_set_param(material, name, value)`: atualiza parametro de material.
- [ ] `render_to_texture(size, draw_fn) -> Texture`: renderiza fora da tela.

N3:

- [ ] `post_process(texture, shader) -> Texture`: aplica efeito fullscreen.
- [ ] `effect_chain(texture, shaders[]) -> Texture`: aplica varios efeitos em cadeia.
- [ ] `light2d_create(config) -> Light`: cria luz 2D.
- [ ] `lighting_apply(scene, lights[])`: aplica iluminacao na cena.
- [ ] `palette_swap(texture, palette)`: troca paleta de cores.

## Movimentacao

N1:

- [ ] `move(entity, delta)`: move entidade por deslocamento.
- [ ] `set_position(entity, position)`: define posicao absoluta.
- [ ] `set_rotation(entity, angle)`: define rotacao.
- [ ] `move_towards(current, target, speed, dt)`: move gradualmente ao alvo.

N2:

- [ ] `dash(entity, direction, speed, duration)`: executa dash.
- [ ] `jump(entity, force)`: aplica salto.
- [ ] `follow_target(entity, target, speed)`: segue alvo.
- [ ] `arrive(entity, target, slow_radius)`: aproxima com desaceleracao.

N3:

- [ ] `controller_top_down_create(config) -> Controller`: cria controlador top-down.
- [ ] `controller_platformer_create(config) -> Controller`: cria controlador plataforma.
- [ ] `controller_update(controller, input, dt)`: atualiza controlador.
- [ ] `controller_set_coyote_time(controller, time)`: aplica coyote time.
- [ ] `controller_set_jump_buffer(controller, time)`: aplica buffer de pulo.
- [ ] `avoid_obstacles(entity, steering)`: desvia obstaculos.

## IA e Comportamento

N1:

- [ ] `look_at(entity, target)`: orienta entidade para alvo.
- [ ] `distance(a, b) -> float`: calcula distancia.

N2:

- [ ] `follow_path(entity, path, speed)`: segue caminho.
- [ ] `wander(entity, radius)`: movimento aleatorio controlado.
- [ ] `seek(entity, target)`: aproxima do alvo.
- [ ] `flee(entity, target)`: afasta do alvo.

N3:

- [ ] `fsm_create() -> FSM`: cria maquina de estados.
- [ ] `fsm_set_state(fsm, state)`: troca estado da FSM.
- [ ] `bt_create() -> BT`: cria behavior tree.
- [ ] `ai_update(ai, dt)`: atualiza logica de IA.
- [ ] `vision_cone(entity, angle, distance) -> Entity[]`: detecta entidades no campo de visao.
- [ ] `hearing_sensor(entity, radius) -> Event[]`: detecta eventos proximos.

## Camera

N1:

- [ ] `camera_set_position(position)`: define posicao da camera.
- [ ] `camera_set_zoom(value)`: define zoom da camera.
- [ ] `world_to_screen(position) -> Vector2`: converte mundo para tela.

N2:

- [ ] `camera_follow(target, smooth)`: segue alvo com suavizacao.
- [ ] `camera_set_bounds(rect)`: limita movimento da camera.
- [ ] `camera_lerp_to(target, speed)`: interpola camera ate alvo.

N3:

- [ ] `camera_rig_create(config) -> CameraRig`: cria rig completo de camera.
- [ ] `camera_shake(intensity, duration)`: aplica tremor.
- [ ] `camera_set_deadzone(rect)`: define zona morta.
- [ ] `camera_set_look_ahead(offset)`: define antecipacao de movimento.
- [ ] `camera_frame_targets(entities[])`: enquadra varios alvos.

## Input

N1:

- [ ] `key_down(key) -> bool`: tecla pressionada no momento.
- [ ] `key_pressed(key) -> bool`: tecla acionada no frame.
- [ ] `axis_get(name) -> float`: valor de eixo virtual.

N2:

- [ ] `action_bind(name, inputs[])`: associa acao a entradas.
- [ ] `action_pressed(name) -> bool`: verifica acao no frame.

N3:

- [ ] `input_context_create(name)`: cria contexto de input.
- [ ] `input_context_push(name)`: ativa contexto de input.
- [ ] `input_rebind(action, input)`: remapeia acao para entrada.
- [ ] `input_record() -> Frame`: grava input do frame.
- [ ] `input_playback(frame)`: reproduz input gravado.

## Mundo e Tilemap

N1:

- [ ] `map_load(file) -> Map`: carrega mapa.
- [ ] `map_draw(map)`: desenha mapa.

N2:

- [ ] `tile_at(map, x, y)`: retorna tile em coordenada.
- [ ] `world_to_tile(position)`: converte mundo para tile.

N3:

- [ ] `map_auto_tile(map)`: ajusta tiles por vizinhanca.
- [ ] `map_build_collision(map)`: gera colisao do mapa.
- [ ] `map_build_navmesh(map)`: gera navegacao para IA.

## Particulas

N1:

- [ ] `particle_emit(position)`: emite particulas.
- [ ] `particle_emitter_update(dt)`: atualiza emissor.

N3:

- [ ] `particle_attach_emitter(entity)`: vincula emissor a entidade.
- [ ] `particle_emit_on_collision(info)`: emite particulas em colisao.
- [ ] `particle_use_preset(name)`: aplica preset pronto.

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
