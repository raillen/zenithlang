# Borealis API Baseline v1

Data: 2026-04-22  
Escopo: baseline de `R3.B0` ate `R3.B7` (parcial).

## borealis.game (camada facil)

Tipos:

- `game.Backend`
- `game.WindowConfig`
- `game.GameContext`
- `game.Color`
- `game.Rect`
- `game.Key`

Loop facil:

- `game.start(config)`
- `game.running(ctx)`
- `game.frame_begin(ctx, clear)`
- `game.frame_end(ctx)`
- `game.close(ctx)`

Draw base:

- `game.draw_rect(ctx, area, color)`
- `game.draw_line(ctx, x1, y1, x2, y2, color)`
- `game.draw_rect_outline(ctx, area, color, thickness)`
- `game.draw_circle(ctx, center, radius, color)`
- `game.draw_circle_outline(ctx, center, radius, color, thickness)`
- `game.draw_text(ctx, value, x, y, size, color)`

Input v1:

- `game.key_down(ctx, key)` implementado
- `game.key_pressed(ctx, key)` implementado (transicao por frame)
- `game.key_released(ctx, key)` implementado (transicao por frame)

Helpers:

- `game.rgb(r, g, b)`
- `game.rgba(r, g, b, a)`
- `game.rect(x, y, width, height)`
- `game.vec2(x, y)`

Scene e Entities v1 (submodulos):

- `borealis.game.entities`
  - `entity_create`, `entity_destroy`, `entity_exists`
  - `entity_set_name`, `entity_get_name`, `entity_find`, `entity_count`, `entity_at`
  - `entity_set_position`, `entity_get_position`
  - `entity_set_rotation`, `entity_get_rotation`
  - `entity_set_scale`, `entity_get_scale`
  - `entity_add_tag`, `entity_remove_tag`, `entity_has_tag`
  - `entity_set_parent`, `entity_remove_parent`, `entity_parent`, `entity_child_count`, `entity_child_at`
  - `entity_snapshot`, `entity_restore_snapshot`, `entity_clone`, `entity_inspect`
- `borealis.game.scene`
  - `scene_create`, `scene_destroy`, `scene_exists`
  - `scene_set_name`, `scene_get_name`, `scene_find`, `scene_count`, `scene_at`
  - `scene_enter`, `scene_exit`, `scene_transition`, `scene_active`
  - `scene_add_entity`, `scene_remove_entity`, `scene_entity_count`, `scene_entity_at`
  - `scene_update`, `scene_draw`, `scene_update_ticks`, `scene_draw_ticks`, `scene_last_delta`

## borealis.engine (camada tecnica)

Tipos:

- `engine.Backend`
- `engine.WindowConfig`
- `engine.Window`
- `engine.Color`
- `engine.Rect`
- `engine.Key`

Controle explicito:

- `engine.open_window(config)`
- `engine.window_should_close(window)`
- `engine.begin_frame(window, clear)`
- `engine.end_frame(window)`
- `engine.close_window(window)`

Draw e input:

- `engine.draw_rect(window, area, color)`
- `engine.draw_line(window, x1, y1, x2, y2, color)`
- `engine.draw_rect_outline(window, area, color, thickness)`
- `engine.draw_circle(window, center, radius, color)`
- `engine.draw_circle_outline(window, center, radius, color, thickness)`
- `engine.draw_text(window, value, x, y, size, color)`
- `engine.key_down(window, key)`
- `engine.key_pressed(window, key)`
- `engine.key_released(window, key)`

Helpers:

- `engine.backend_code(backend)`
- `engine.key_code(key)`
- `engine.rgb(r, g, b)`
- `engine.rgba(r, g, b, a)`
- `engine.rect(x, y, width, height)`
- `engine.vec2(x, y)`

ECS hibrido (subset inicial):

- `borealis.engine.ecs`
  - `ecs_component_set(entity_id, component_type, payload)`
  - `ecs_component_remove(entity_id, component_type)`
  - `ecs_component_has(entity_id, component_type)`
  - `ecs_component_get(entity_id, component_type)`
  - `ecs_component_count(entity_id)`
  - `ecs_component_list_blob(entity_id)`
  - `ecs_system_register(name, phase)`
  - `ecs_system_destroy(system)`
  - `ecs_system_enable(system, enabled)`
  - `ecs_system_is_enabled(system)`
  - `ecs_system_mark_run(system, dt)`
  - `ecs_system_info(system)`
  - `ecs_system_count()`
  - `ecs_system_at(index)`

Facade simples na camada facil:

- `borealis.game.entities`
  - `entity_add_component(entity, component_type, payload)`
  - `entity_remove_component(entity, component_type)`
  - `entity_has_component(entity, component_type)`
  - `entity_get_component(entity, component_type)`
  - `entity_component_count(entity)`
  - `entity_list_components_blob(entity)`

B6 scaffolds modulares (inicial):

- `borealis.game.contracts`: `FrameContext`, `ActionInput`, `Body2D`, `action_axis`, `body_integrate`
- `borealis.game.movement`: `move`, `push`, `teleport`, `move_towards`
- `borealis.game.controllers`: `direction_from_input`, `topdown_update`, `grid_step`
- `borealis.game.vehicles`: `car_create`, `car_update`
- `borealis.game.animation`: `animation_create`, `animation_play`, `animation_stop`, `animation_update`
- `borealis.game.audio`: `bus_default`, `bus_set_master`, `bus_set_music`, `bus_set_sfx`, `sound_play`, `music_play`
- `borealis.game.ai`: `seek`, `evade`, `move_randomly`
- `borealis.game.camera`: `create`, `follow`, `apply_bounds`, `shake_offset`
- `borealis.game.input`: `key_state`, `mouse_hover`
- `borealis.game.world`: `set_solid`, `is_solid`, `set_walkable`, `is_walkable`
- `borealis.game.procedural`: `set_seed`, `get_seed`, `next_i32`, `range_int`
- `borealis.game.ui`: `point_in_rect`, `button`, `label`
- `borealis.game.ui.hud`: `defaults`, `set_health`, `add_score`
- `borealis.game.assets`: `asset_load_image`, `asset_load_text`, `asset_load_binary`, `asset_unload`, `asset_is_loaded`
- `borealis.game.save`: `slot_write`, `slot_read`, `slot_delete`, `autosave_write`, `autosave_read`
- `borealis.game.storage`: `write`, `read`, `remove`, `exists`
- `borealis.game.database`: `open`, `close`, `query`
- `borealis.game.services`: `http_send`, `cloud_connect`, `cloud_is_connected`
- `borealis.game.settings`: `defaults`, `set_language`, `set_master_volume`, `set_dyslexia_font`
- `borealis.game.events`: `emit`, `last_name`, `last_payload`, `count`
- `borealis.game.debug`: `set_enabled`, `is_enabled`, `watch_text`, `hitbox_text`
- `borealis.game.editor`: `set/get label`, `set/get note`, `set/get group`, `set/is locked`, `set/is hidden`, `inspect`

## borealis (compat raiz)

Mantido para compatibilidade no ciclo atual, com janela/frame/draw/input e primitives de desenho base.

## Runtime C (hook desktop B7)

Contrato C adicional para backend desktop:

- `zt_borealis_desktop_api`
- `zt_borealis_set_desktop_api(api)`
- `zt_borealis_get_desktop_api()`
- adapter Raylib inicial builtin por carga dinamica de biblioteca

Regra de fallback:

- quando `backend_id=1` for solicitado sem adapter registrado, o runtime faz fallback seguro para stub.
