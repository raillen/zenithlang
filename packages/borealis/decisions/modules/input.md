# Borealis Module Decision - Input

- Status: proposed
- Date: 2026-04-22
- Type: module / input API
- Scope: `borealis.game.input`

## Summary

Input handles raw device state, action mapping, mouse helpers, and optional contexts.

## Implementation

- keep raw input and action input both available;
- use explicit names for pressed/down/released;
- allow optional context stacks for menus and gameplay states.

## Proposed API

- `key_down(key) -> bool`: checks whether a key is held.
- `key_pressed(key) -> bool`: checks whether a key was pressed this frame.
- `key_released(key) -> bool`: checks whether a key was released this frame.
- `axis_get(name) -> float`: reads a virtual axis.
- `action_bind(name, inputs[])`: binds inputs to an action.
- `action_pressed(name) -> bool`: checks whether an action was pressed.
- `action_down(name) -> bool`: checks whether an action is held.
- `action_released(name) -> bool`: checks whether an action was released.
- `mouse_position() -> Vector2`: reads mouse position.
- `mouse_delta() -> Vector2`: reads mouse movement delta.
- `mouse_button_down(button) -> bool`: checks mouse button hold.
- `mouse_button_pressed(button) -> bool`: checks mouse button press.
- `mouse_button_released(button) -> bool`: checks mouse button release.
- `mouse_hover(target) -> bool`: checks if the mouse is over a target.
- `input_context_create(name)`: creates a context.
- `input_context_push(name)`: pushes a context.
- `input_context_pop()`: pops the current context.
- `input_context_clear()`: clears the context stack.
- `input_rebind(action, input)`: changes a binding.
- `input_record() -> Frame`: records input state.
- `input_playback(frame)`: replays recorded input.
- `input_lock()`: locks input temporarily.
- `input_unlock()`: unlocks input.
- `gamepad_button_down(button) -> bool`: checks a gamepad button.
- `gamepad_button_pressed(button) -> bool`: checks a gamepad press.
- `gamepad_axis_get(name) -> float`: reads a gamepad axis.
- `text_input_get() -> text`: reads typed text.
- `input_any_pressed() -> bool`: checks if anything was pressed.
- `input_any_released() -> bool`: checks if anything was released.
- `input_set_deadzone(value)`: sets analog deadzone.
- `input_set_repeat_delay(value)`: sets repeat delay.
- `input_set_repeat_rate(value)`: sets repeat rate.
- `input_set_mouse_locked(bool)`: locks or unlocks mouse.
- `input_set_cursor_visible(bool)`: shows or hides cursor.

## Notes

- `mouse_hover` can be implemented through collision or area checks.
- `Vector2` is used for spatial values.
