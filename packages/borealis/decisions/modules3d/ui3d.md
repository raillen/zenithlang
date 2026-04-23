# Borealis Module Decision - UI3D

- Status: proposed
- Date: 2026-04-22
- Type: module / 3D interface API
- Scope: `borealis.game.ui3d`

## Summary

UI3D provides simple interface elements that live in 3D space.
It complements the regular HUD and menu flow from `borealis.game.ui`.

## Implementation

- keep world-space UI and screen-projected UI as separate concerns;
- cover common gameplay needs first: labels, bars, prompts, and buttons;
- avoid heavy layout or web-like UI systems in v1;
- keep billboarding, binding, and projection helpers built in.

## Recommended structure

- `borealis.game.ui3d.world`
- `borealis.game.ui3d.screen`

## Proposed API

### Element creation

- `ui3d_create_label(text: text, position: game.Vector3) -> result<ui3d.Ui3dId, core.Error>`: creates a world-space label.
- `ui3d_create_bar(position: game.Vector3, size: game.Vector2) -> result<ui3d.Ui3dId, core.Error>`: creates a world-space bar, such as health or mana.
- `ui3d_create_prompt(text: text, position: game.Vector3) -> result<ui3d.Ui3dId, core.Error>`: creates an interaction prompt.
- `ui3d_create_panel(position: game.Vector3, size: game.Vector2) -> result<ui3d.Ui3dId, core.Error>`: creates a simple world panel.
- `ui3d_create_button(text: text, position: game.Vector3, size: game.Vector2) -> result<ui3d.Ui3dId, core.Error>`: creates an interactive world button.

### Lifecycle and visibility

- `ui3d_destroy(id: ui3d.Ui3dId) -> result<void, core.Error>`: removes an element.
- `ui3d_exists(id: ui3d.Ui3dId) -> bool`: checks if an element exists.
- `ui3d_set_enabled(id: ui3d.Ui3dId, enabled: bool) -> result<void, core.Error>`: toggles element update state.
- `ui3d_is_enabled(id: ui3d.Ui3dId) -> bool`: reads enabled state.
- `ui3d_set_visible(id: ui3d.Ui3dId, visible: bool) -> result<void, core.Error>`: toggles visibility.
- `ui3d_is_visible(id: ui3d.Ui3dId) -> bool`: reads visibility.

### Transform and presentation

- `ui3d_set_position(id: ui3d.Ui3dId, position: game.Vector3) -> result<void, core.Error>`: sets world position.
- `ui3d_get_position(id: ui3d.Ui3dId) -> game.Vector3`: reads world position.
- `ui3d_set_size(id: ui3d.Ui3dId, size: game.Vector2) -> result<void, core.Error>`: sets element size.
- `ui3d_get_size(id: ui3d.Ui3dId) -> game.Vector2`: reads element size.
- `ui3d_set_scale(id: ui3d.Ui3dId, scale: game.Vector3) -> result<void, core.Error>`: sets element scale.
- `ui3d_get_scale(id: ui3d.Ui3dId) -> game.Vector3`: reads element scale.
- `ui3d_face_camera(id: ui3d.Ui3dId, enabled: bool) -> result<void, core.Error>`: makes the element face the camera.
- `ui3d_is_facing_camera(id: ui3d.Ui3dId) -> bool`: reads billboarding state.

### Content

- `ui3d_set_text(id: ui3d.Ui3dId, text: text) -> result<void, core.Error>`: sets element text.
- `ui3d_get_text(id: ui3d.Ui3dId) -> text`: reads element text.
- `ui3d_set_value(id: ui3d.Ui3dId, value: float) -> result<void, core.Error>`: sets bar value (`0` to `1`).
- `ui3d_get_value(id: ui3d.Ui3dId) -> float`: reads bar value.
- `ui3d_set_color(id: ui3d.Ui3dId, color: game.Color) -> result<void, core.Error>`: sets primary color.
- `ui3d_get_color(id: ui3d.Ui3dId) -> game.Color`: reads primary color.

### Entity binding

- `ui3d_bind_entity(id: ui3d.Ui3dId, entity: entities.Entity, offset: game.Vector3) -> result<void, core.Error>`: binds UI to an entity.
- `ui3d_get_entity(id: ui3d.Ui3dId) -> optional<entities.Entity>`: reads bound entity.
- `ui3d_unbind_entity(id: ui3d.Ui3dId) -> result<void, core.Error>`: clears entity binding.

### Interaction

- `ui3d_hit_test(ray: game.Ray3) -> optional<ui3d.Ui3dId>`: tests which element was hit by a ray.
- `ui3d_hovered(ray: game.Ray3) -> optional<ui3d.Ui3dId>`: returns hovered element.
- `ui3d_pressed(ray: game.Ray3) -> optional<ui3d.Ui3dId>`: returns pressed element.
- `ui3d_set_interactable(id: ui3d.Ui3dId, enabled: bool) -> result<void, core.Error>`: toggles interaction.
- `ui3d_is_interactable(id: ui3d.Ui3dId) -> bool`: reads interaction state.

### Screen bridge

- `ui3d_world_to_screen(position: game.Vector3) -> game.Point2`: projects a world point to the screen.
- `ui3d_is_on_screen(position: game.Vector3) -> bool`: checks if a world point is visible on screen.
- `ui3d_anchor_to_screen(id: ui3d.Ui3dId, mode: ui3d.AnchorMode) -> result<void, core.Error>`: anchors the element as a projected screen element.
- `ui3d_get_screen_rect(id: ui3d.Ui3dId) -> game.Rect`: gets the current screen-space rect.

### Helpers

- `ui3d_set_distance_fade(id: ui3d.Ui3dId, start: float, end: float) -> result<void, core.Error>`: fades UI by distance.
- `ui3d_set_occlusion(id: ui3d.Ui3dId, enabled: bool) -> result<void, core.Error>`: hides UI when occluded by world geometry.
- `ui3d_set_min_scale(id: ui3d.Ui3dId, value: float) -> result<void, core.Error>`: prevents becoming too small.
- `ui3d_set_max_scale(id: ui3d.Ui3dId, value: float) -> result<void, core.Error>`: prevents becoming too large.
- `ui3d_debug_info(id: ui3d.Ui3dId) -> ui3d.Ui3dDebugInfo`: returns debug/editor-friendly state.

## Notes

- `ui3d` is for world-space or projected 3D interface.
- regular HUD and menu flow stay in `borealis.game.ui`.
- v1 should prioritize `label`, `bar`, `prompt`, and `button`.

