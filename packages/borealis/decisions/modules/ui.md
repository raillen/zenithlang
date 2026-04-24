# Borealis Module Decision - UI

- Status: accepted
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

- `button(...)`: returns immediate button state.
- `label(...)`: returns plain label text for simple HUD/examples.
- `panel(...)`: creates a panel widget.
- `label_widget(...)`: creates a label widget.
- `checkbox(...)`: creates a checkbox widget.
- `slider(...)`: creates a slider widget with clamped value.
- `text_input(...)`: creates a text input widget.
- `image(...)`: creates an image widget bound to an asset key.
- `container(...)`: creates a container widget.
- `layout_vertical(...)` and `layout_row(...)`: apply simple linear layout rules.
- `hover(...)` and `click(...)`: check pointer interaction.
- `state(...)`: combines hover, press, click, focus, and visibility.
- `focus(...)`: manages focus.
- `show(...)`, `hide(...)`, and `set_visible(...)`: control visibility.
- `set_value(...)`, `set_checked(...)`, and `set_text(...)`: update widget state.

## HUD Namespace

- `borealis.game.ui.hud.add_widget(...)`: adds a HUD widget.
- `borealis.game.ui.hud.remove_widget(...)`: removes a HUD widget.
- `borealis.game.ui.hud.show(...)`: shows an overlay widget.
- `borealis.game.ui.hud.hide(...)`: hides an overlay widget.
- `borealis.game.ui.hud.set_visible(...)`: toggles visibility.
- `borealis.game.ui.hud.set_position(...)`: positions the overlay.
- `borealis.game.ui.hud.set_value(...)`: changes a HUD value.
- `borealis.game.ui.hud.set_text(...)`: changes HUD text.
- `borealis.game.ui.hud.inspect(...)`: reads widget text, value, position, and visibility.

## Notes

- UI is for menus, panels, dialogs, inventory, and controls.
- HUD is for in-game overlay data only.
