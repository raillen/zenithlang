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
  - `scene_set_phase`, `scene_get_phase`, `scene_set_document_id`, `scene_get_document_id`
  - `scene_enter`, `scene_exit`, `scene_transition`, `scene_active`
  - `scene_push`, `scene_pop`, `scene_stack_count`, `scene_stack_at`, `scene_stack_top`
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
- `borealis.game.ui`: `widget`, `panel`, `label_widget`, `checkbox`, `slider`, `text_input`, `image`, `container`, `point_in_rect`, `hover`, `click`, `button`, `state`, `focus`, `show`, `hide`, `set_visible`, `set_value`, `set_checked`, `set_text`, `layout_vertical`, `layout_row`, `label`
- `borealis.game.ui.hud`: `defaults`, `set_health`, `add_score`, `add_widget`, `remove_widget`, `has_widget`, `widget_count`, `show`, `hide`, `set_visible`, `is_visible`, `set_position`, `position`, `set_value`, `get_value`, `set_text`, `get_text`, `inspect`, `reset`
- `borealis.game.assets`: `asset_load`, `asset_load_from`, `asset_load_image`, `asset_load_texture`, `asset_load_text`, `asset_load_binary`, `asset_load_font`, `asset_load_sound`, `asset_load_music`, `asset_load_shader`, `asset_load_atlas`, `asset_load_spritesheet`, `asset_get`, `asset_get_kind`, `asset_find`, `asset_find_kind`, `asset_find_id`, `asset_exists_kind`, `asset_info`, `asset_info_key`, `asset_info_id`, `asset_unload`, `asset_unload_key`, `asset_unload_id`, `asset_reload`, `asset_reload_key`, `asset_reload_id`, `asset_cache_clear`, `asset_count`, `asset_total_count`, `asset_count_kind`, `asset_total_count_kind`, `asset_count_source`, `asset_total_count_source`, `asset_exists`, `asset_is_loaded`, `asset_is_loaded_key`
- `borealis.game.save`: `slot_write`, `slot_read`, `slot_delete`, `slot_exists`, `autosave_write`, `autosave_read`, `autosave_delete`, `snapshot_write`, `snapshot_read`, `snapshot_restore`, `snapshot_delete`, `slot_field_write`, `slot_field_read`, `slot_field_delete`, `slot_field_exists`, `slot_field_count`, `slot_field_clear`, `autosave_field_write`, `autosave_field_read`, `autosave_field_delete`, `autosave_field_exists`, `autosave_field_count`, `autosave_field_clear`, `reset`
- `borealis.game.storage`: `write`, `read`, `remove`, `exists`, `copy`, `move`, `clear`
- `borealis.game.database`: `open`, `close`, `is_open`, `exec`, `query`, `last_sql`, `reset`
- `borealis.game.services`: `http_send`, `cloud_connect`, `cloud_disconnect`, `cloud_is_connected`, `cloud_upload`, `cloud_download`, `cloud_remove`, `cloud_exists`, `cloud_entry_count`, `request_count`, `last_url`, `reset`
- `borealis.game.settings`: `defaults`, `current_state`, `apply_defaults`, `apply_state`, `set_language`, `set_master_volume`, `set_music_volume`, `set_sfx_volume`, `set_dyslexia_font`, `set_high_contrast`, `set_reduce_motion`, `set_fullscreen`, `set_vsync`, `set_render_scale`, `set_ui_scale`, `save_profile`, `load_profile` (`optional`), `apply_profile` (`optional`), `delete_profile`, `profile_exists`, `reset`, `reset_all`, `list_names`, `get_text`, `get_number`, `get_flag`
- `borealis.game.events`: `emit`, `queue`, `dispatch`, `on`, `once`, `off`, `clear`, `is_registered`, `listener_count`, `peek`, `poll`, `dispatch_next`, `listener_peek`, `listener_poll`, `clear_pending`, `clear_listener`, `has_pending`, `pending_count`, `listener_has_pending`, `listener_pending_count`, `count_named`, `reset`, `last_name`, `last_payload`, `count`, `event_message`, `event_name`, `event_payload`
- `borealis.game.debug`: `set_enabled`, `is_enabled`, `log`, `log_count`, `last_log`, `set_flag`, `toggle`, `flag_enabled`, `flag_count`, `watch`, `unwatch`, `watch_count`, `watch_value`, `watch_text`, `watch_entry`, `grid_text`, `bounds_text`, `hitbox_text`, `fps_text`, `overlay_summary`, `clear`
- `borealis.game.editor`: `set/get label`, `set/get note`, `set/get group`, `set/is locked`, `set/is hidden`, `inspect`, `has_meta`, `clear`, `reset`

Observacao de comportamento:

- `save.slot_read(...)` retorna `none` quando o slot foi apagado
- `save.slot_read(...)` preserva `""` como valor valido quando o slot existe
- `save.slot_field_read(...)` retorna `none` quando o campo foi apagado e preserva `""` como valor valido
- `storage.read(...)` retorna `none` quando a chave foi removida
- `storage.read(...)` preserva `""` como valor valido quando a chave existe
- `assets` agora diferencia asset registrado de asset carregado, preserva `id` estavel em `asset_reload(...)`, expoe metadata leve com origem e stamp logico, oferece loaders especificos para os tipos 2D mais comuns e rejeita chave simples ambigua com `kind` diferente
- `services.http_send(...)` devolve resposta stub previsivel e rastreia a ultima URL usada
- `services.cloud_*` usa estado em memoria por provider e preserva `""` como valor remoto valido
- `database.query(...)` devolve payload stub previsivel e `database.last_sql(...)` rastreia o ultimo SQL da conexao
- `settings` agora cobre audio, idioma, acessibilidade, fullscreen, `vsync`, `render_scale`, `ui_scale` e perfis persistidos em `storage`
- `events` agora combina fila global com listeners nomeados, `once`, rotas desacopladas e introspeccao por listener
- `debug` agora cobre logs, flags, watches e overlays textuais para grid, bounds, hitbox e FPS
- `ui` agora cobre widgets simples, foco, hover/click, visibilidade e layout linear
- `ui.hud` agora cobre widgets de HUD com texto, valor, posicao e visibilidade
- `scene` agora cobre fase, document id e pilha simples de fluxo
- `editor` agora cobre limpeza/reset de metadata por stable id

## borealis (compat raiz)

Mantido para compatibilidade no ciclo atual, com janela/frame/draw/input e primitives de desenho base.

## borealis.raylib (binding direto inicial)

Tipos:

- `raylib.Window`
- `raylib.Color`
- `raylib.Rect`
- `raylib.Vector2`
- `raylib.Texture`
- `raylib.Sound`

Disponibilidade e diagnostico:

- `raylib.available()`
- `raylib.loaded_path()`
- `raylib.require_available()`

Janela/frame:

- `raylib.open_window(...)`
- `raylib.open_window_required(...)`
- `raylib.close_window(window)`
- `raylib.window_should_close(window)`
- `raylib.begin_drawing(window, clear)`
- `raylib.end_drawing(window)`

Shapes e texto:

- `raylib.draw_rectangle(window, area, color)`
- `raylib.draw_rectangle_lines(window, area, color, thickness)`
- `raylib.draw_line(window, start, finish, color)`
- `raylib.draw_triangle(window, p1, p2, p3, color)`
- `raylib.draw_circle(window, center, radius, color)`
- `raylib.draw_circle_lines(window, center, radius, color, thickness)`
- `raylib.draw_ellipse(window, center, radius_h, radius_v, color)`
- `raylib.draw_text(window, value, x, y, size, color)`
- `raylib.measure_text(value, font_size)`

Textura e audio:

- `raylib.load_texture(path)` com erro claro para caminho vazio
- `raylib.unload_texture(texture)`
- `raylib.draw_texture(window, texture, position, tint)`
- `raylib.draw_texture_ex(window, texture, position, rotation, scale_amount, tint)`
- `raylib.init_audio_device()`
- `raylib.close_audio_device()`
- `raylib.audio_device_ready()`
- `raylib.set_master_volume(volume)`
- `raylib.load_sound(path)` com erro claro para caminho vazio
- `raylib.unload_sound(sound)`
- `raylib.play_sound(sound)`
- `raylib.stop_sound(sound)`
- `raylib.set_sound_volume(sound, volume)`

Input, cor e math:

- `raylib.is_key_down(window, key)`
- `raylib.is_key_pressed(window, key)`
- `raylib.is_key_released(window, key)`
- `raylib.rgb(...)`, `raylib.rgba(...)`, `raylib.white()`, `raylib.black()`, `raylib.red()`, `raylib.green()`, `raylib.blue()`, `raylib.yellow()`
- `raylib.rect(...)`, `raylib.vec2(...)`
- `raylib.key_left()`, `raylib.key_right()`, `raylib.key_up()`, `raylib.key_down()`, `raylib.key_space()`, `raylib.key_escape()`
- `raylib.vector_length(...)`, `raylib.vector_distance(...)`, `raylib.lerp(...)`
- `raylib.ease_linear(...)`, `raylib.ease_sine_in/out/in_out(...)`, `raylib.ease_quad_in/out/in_out(...)`

## Runtime C (hook desktop B7)

Contrato C adicional para backend desktop:

- `zt_borealis_desktop_api`
- `zt_borealis_set_desktop_api(api)`
- `zt_borealis_get_desktop_api()`
- adapter Raylib inicial builtin por carga dinamica de biblioteca

Regra de fallback:

- quando `backend_id=1` for solicitado sem adapter registrado, o runtime faz fallback seguro para stub.
