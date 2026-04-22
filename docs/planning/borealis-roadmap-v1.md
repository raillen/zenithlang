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

Status: proposto
Data: 2026-04-22
Base: skeleton Borealis + stub runtime + linker hook

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

## R3.B3 - Render2D base

Objetivo:

- entregar desenho util para prototipos 2D.

Entregas:

- `line`, `rect`, `rect_outline`, `circle`, `circle_outline`, `text`;
- helpers de cor/area consistentes;
- exemplos com HUD e movimento basico.

## R3.B4 - Scene e Entity v1

Objetivo:

- introduzir organizacao de codigo sem complexidade excessiva.

Entregas:

- API minima de `scene` e `entity`;
- spawn/remove/listagem de entidades;
- desenho por cena e atualizacao por frame.

## R3.B5 - ECS hibrido (interno)

Objetivo:

- preparar crescimento tecnico sem expor complexidade cedo.

Entregas:

- componentes e sistemas no `engine` (subset oficial);
- facade simples no `game` para uso padrao;
- guidelines de quando sair da camada facil.

## R3.B6 - Subsistemas modulares (scaffold)

Objetivo:

- preparar arquitetura para expansao e editor futuro.

Entregas:

- scaffolds de `physics2d`, `audio`, `controls`, `assets`, `time`;
- contratos minimos de integracao entre subsistemas;
- metadados iniciais para futura integracao com editor.

## R3.B7 - Backend desktop inicial

Objetivo:

- validar backend real apos stub.

Entregas:

- adaptador desktop inicial (Raylib/OpenGL) com linker profile documentado;
- fallback seguro para stub quando backend real nao estiver ligado;
- teste E2E de janela + input + draw.

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

## Catalogo funcional por nivel (proposto)

Este catalogo organiza as funcoes por topico e por nivel de implementacao:

- N1 = base (entrada facil, baixo risco)
- N2 = intermediario (comportamento util de gameplay)
- N3 = avancado (game changer, maior custo tecnico)

Observacao de naming em aberto:

- `shape_sweep` e `find_overlaps` sao nomes candidatos para substituir nomes mais genericos (`sweep` e `query`), sem decisao final fechada.

## Colisao

N1:

- `collides(a, b) -> bool`: testa se dois shapes se sobrepoem.
- `contains_point(shape, point) -> bool`: verifica se um ponto esta dentro de um shape.
- `overlap_query(area) -> Entity[]`: retorna entidades dentro de uma area.
- `raycast(origin, direction, max_distance) -> Hit`: retorna o primeiro impacto ao longo de um raio.
- `linecast(start, end) -> Hit`: versao simplificada de raycast com ponto inicial e final.
- `shape_sweep(shape, motion) -> Hit`: detecta colisao ao deslocar um shape pelo vetor de movimento.

N2:

- `find_nearest(shape, radius) -> Entity`: retorna a entidade mais proxima em um raio.
- `find_overlaps(shape, filter) -> Entity[]`: busca entidades que sobrepoem o shape com filtro customizado.
- `collision_normal(hit) -> Vector2`: retorna a normal da colisao encontrada.
- `separation_vector(a, b) -> Vector2`: retorna o menor vetor para separar dois corpos.

N3:

- `move_and_collide(entity, motion) -> CollisionInfo`: move entidade e retorna dados completos de colisao.
- `move_and_slide(entity, velocity, up) -> MotionResult`: move respeitando superficies e inclinacoes.
- `shape_cast(shape, direction, distance) -> Hit`: raycast com volume do shape.
- `predict_collision(entity, motion) -> Hit`: antecipa colisao antes do movimento real.
- `get_contacts(entity) -> Contact[]`: lista contatos ativos da entidade no frame.

## Fisica

N1:

- `create_body(body_type) -> Body`: cria corpo fisico.
- `set_velocity(body, velocity)`: define velocidade linear.
- `add_force(body, force)`: aplica forca continua.
- `add_impulse(body, impulse)`: aplica impulso instantaneo.
- `set_gravity_scale(body, scale)`: ajusta quanto o corpo recebe gravidade.
- `set_mass(body, mass)`: define massa do corpo.
- `physics_step(dt)`: avanca simulacao fisica.

N2:

- `set_drag(body, drag)`: define resistencia de movimento.
- `set_friction(body, friction)`: define atrito em contatos.
- `set_bounciness(body, value)`: define quique/restituicao.
- `lock_rotation(body, locked)`: trava rotacao do corpo.

N3:

- `character_body(config) -> Character`: cria corpo otimizado para personagem.
- `move_character(character, input, dt) -> MotionResult`: executa movimento de personagem com colisao.
- `is_on_floor(character) -> bool`: verifica contato com chao.
- `is_on_wall(character) -> bool`: verifica contato lateral.
- `apply_knockback(body, direction, force)`: aplica recuo de gameplay.
- `rewind_physics(state)`: restaura estado anterior da simulacao.

## Sprites

N1:

- `sprite_draw(texture, position)`: desenha textura em uma posicao.
- `sprite_draw_ex(texture, transform)`: desenha textura com rotacao/escala.
- `sprite_draw_region(texture, source, dest)`: desenha uma regiao da textura.
- `sprite_set_color(sprite, color)`: aplica cor/tint no sprite.
- `sprite_set_flip(sprite, flip_x, flip_y)`: espelha sprite nos eixos.

N2:

- `anim_create(frames, fps, loop) -> Animation`: cria animacao por frames.
- `anim_play(sprite, animation)`: inicia animacao no sprite.
- `anim_update(sprite, dt)`: avanca animacao com delta time.
- `anim_set_speed(sprite, speed)`: ajusta velocidade da animacao.

N3:

- `animator_create() -> Animator`: cria maquina de estados de animacao.
- `animator_set_state(animator, state)`: troca estado atual da animacao.
- `anim_sync_with_velocity(sprite, velocity)`: sincroniza animacao com velocidade.
- `anim_directional(sprite, direction)`: escolhe animacao por direcao.
- `sprite_batch_create() -> Batch`: cria sistema de desenho em lote.

## Shaders e FX

N1:

- `shader_create(vs, fs) -> Shader`: cria shader por codigo fonte.
- `shader_set_uniform(shader, name, value)`: define valor de uniform.
- `shader_apply(target, shader)`: aplica shader em alvo de render.

N2:

- `material_create(shader) -> Material`: encapsula shader com parametros.
- `material_set_param(material, name, value)`: atualiza parametro de material.
- `render_to_texture(size, draw_fn) -> Texture`: renderiza offscreen para textura.

N3:

- `post_process(texture, shader) -> Texture`: aplica efeito fullscreen.
- `effect_chain(texture, shaders[]) -> Texture`: aplica cadeia de efeitos.
- `light2d_create(config) -> Light`: cria luz 2D.
- `lighting_apply(scene, lights[])`: aplica iluminacao da cena.
- `palette_swap(texture, palette)`: troca paleta de cores dinamicamente.

## Movimentacao

N1:

- `move(entity, delta)`: move entidade por deslocamento.
- `set_position(entity, position)`: define posicao absoluta.
- `set_rotation(entity, angle)`: define rotacao da entidade.
- `move_towards(current, target, speed, dt)`: move gradualmente para alvo.

N2:

- `dash(entity, direction, speed, duration)`: executa dash temporario.
- `jump(entity, force)`: aplica impulso de salto.
- `follow_target(entity, target, speed)`: faz entidade seguir alvo.
- `arrive(entity, target, slow_radius)`: aproxima com desaceleracao perto do alvo.

N3:

- `controller_top_down_create(config) -> Controller`: cria controlador top-down.
- `controller_platformer_create(config) -> Controller`: cria controlador de plataforma.
- `controller_update(controller, input, dt)`: atualiza controlador com feel de gameplay.
- `controller_set_coyote_time(controller, time)`: permite pulo curto apos cair da borda.
- `controller_set_jump_buffer(controller, time)`: guarda input de pulo por curto intervalo.
- `avoid_obstacles(entity, steering)`: desvia obstaculos automaticamente.

## IA e Comportamento

N1:

- `look_at(entity, target)`: orienta entidade para olhar alvo.
- `distance(a, b) -> float`: calcula distancia entre pontos/entidades.

N2:

- `follow_path(entity, path, speed)`: percorre caminho predefinido.
- `wander(entity, radius)`: movimento aleatorio controlado.
- `seek(entity, target)`: busca alvo aproximando.
- `flee(entity, target)`: afasta de alvo.

N3:

- `fsm_create() -> FSM`: cria maquina de estados.
- `fsm_set_state(fsm, state)`: troca estado da FSM.
- `bt_create() -> BT`: cria behavior tree.
- `ai_update(ai, dt)`: atualiza logica de IA.
- `vision_cone(entity, angle, distance) -> Entity[]`: detecta entidades no campo de visao.
- `hearing_sensor(entity, radius) -> Event[]`: detecta eventos sonoros proximos.

## Camera

N1:

- `camera_set_position(position)`: define posicao da camera.
- `camera_set_zoom(value)`: define zoom da camera.
- `world_to_screen(position) -> Vector2`: converte coordenadas de mundo para tela.

N2:

- `camera_follow(target, smooth)`: segue alvo com suavizacao.
- `camera_set_bounds(rect)`: limita area de movimento da camera.
- `camera_lerp_to(target, speed)`: interpola camera para alvo.

N3:

- `camera_rig_create(config) -> CameraRig`: cria sistema completo de camera.
- `camera_shake(intensity, duration)`: aplica tremor.
- `camera_set_deadzone(rect)`: define zona morta de acompanhamento.
- `camera_set_look_ahead(offset)`: antecipa direcao do movimento.
- `camera_frame_targets(entities[])`: enquadra varios alvos na tela.

## Input

N1:

- `key_down(key) -> bool`: retorna se tecla esta pressionada no momento.
- `key_pressed(key) -> bool`: retorna se tecla foi acionada no frame atual.
- `axis_get(name) -> float`: retorna valor de eixo virtual.

N2:

- `action_bind(name, inputs[])`: associa uma acao a entradas fisicas.
- `action_pressed(name) -> bool`: verifica se acao foi acionada no frame.

N3:

- `input_context_create(name)`: cria contexto de input.
- `input_context_push(name)`: ativa contexto no topo da pilha.
- `input_rebind(action, input)`: remapeia acao para nova entrada.
- `input_record() -> Frame`: grava estado de input do frame.
- `input_playback(frame)`: reproduz input gravado.

## Mundo e Tilemap

N1:

- `map_load(file) -> Map`: carrega mapa de arquivo.
- `map_draw(map)`: desenha mapa carregado.

N2:

- `tile_at(map, x, y)`: retorna tile em coordenada de grade.
- `world_to_tile(position)`: converte posicao de mundo para coordenada de tile.

N3:

- `map_auto_tile(map)`: ajusta tiles automaticamente por vizinhanca.
- `map_build_collision(map)`: gera dados de colisao a partir do mapa.
- `map_build_navmesh(map)`: gera navegacao para IA.

## Particulas

N1:

- `particle_emit(position)`: emite particula em posicao.
- `particle_emitter_update(dt)`: atualiza emissor de particulas.

N3:

- `particle_attach_emitter(entity)`: vincula emissor a entidade.
- `particle_emit_on_collision(info)`: dispara particulas em evento de colisao.
- `particle_use_preset(name)`: aplica preset pronto de particulas.

## Fora de escopo do Borealis 1.0

- editor visual completo
- tilemap editor completo
- pipeline de importacao de assets avancado
- 3D
- netcode multiplayer completo

## Definicao de pronto por milestone

Uma milestone so fecha quando tiver:

1. API/decision atualizada (quando houver mudanca de comportamento);
2. exemplos e testes minimos da milestone;
3. checklist Borealis com evidencia;
4. sem regressao acima de budget definido para a milestone;
5. docs de uso e limites conhecidos atualizados.
