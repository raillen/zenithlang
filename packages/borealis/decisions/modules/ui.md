# Borealis Module Decision - UI

- Status: proposed
- Date: 2026-04-22
- Type: module / interface API
- Scope: `borealis.game.ui`

## Summary

UI handles general interface widgets, layout, interaction, and the `hud` namespace.

## Implementation

- keep UI simple and composable;
- keep HUD as an in-UI namespace, not as a top-level module;
- avoid mixing gameplay logic into widgets.

## Proposed API

- `ui_button(...)`: creates a button.
- `ui_label(...)`: creates a label.
- `ui_panel(...)`: creates a panel.
- `ui_checkbox(...)`: creates a checkbox.
- `ui_slider(...)`: creates a slider.
- `ui_text_input(...)`: creates a text input.
- `ui_image(...)`: creates an image widget.
- `ui_container(...)`: creates a container.
- `ui_layout(...)`: applies layout rules.
- `ui_hover(...)`: checks hover state.
- `ui_click(...)`: checks click state.
- `ui_focus(...)`: manages focus.
- `ui_show(...)`: shows a widget.
- `ui_hide(...)`: hides a widget.

## HUD Namespace

- `borealis.game.ui.hud.show(...)`: shows an overlay widget.
- `borealis.game.ui.hud.hide(...)`: hides an overlay widget.
- `borealis.game.ui.hud.set_visible(...)`: toggles visibility.
- `borealis.game.ui.hud.set_position(...)`: positions the overlay.
- `borealis.game.ui.hud.add_widget(...)`: adds a HUD widget.
- `borealis.game.ui.hud.remove_widget(...)`: removes a HUD widget.
- `borealis.game.ui.hud.set_value(...)`: changes a HUD value.
- `borealis.game.ui.hud.set_text(...)`: changes HUD text.

## Notes

- UI is for menus, panels, dialogs, inventory, and controls.
- HUD is for in-game overlay data only.
