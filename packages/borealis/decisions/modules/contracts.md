# Module Decision: game.contracts

## Purpose

`borealis.game.contracts` defines shared runtime contracts between gameplay modules.

Goal: let `controllers`, `movement`, `ai`, `camera`, and `input` speak the same data model.

## Initial Scope (N1)

- `FrameContext`: `dt` + frame counter.
- `ActionInput`: directional and action buttons.
- `Body2D`: `position` + `velocity`.

## Functions

- `frame_context(dt, frame)`: create/update frame context.
- `action_input()`: default input object.
- `body2d(position, velocity)`: create body.
- `action_axis(input)`: convert input booleans to axis vector.
- `body_set_velocity(body, velocity)`: write velocity.
- `body_integrate(body, dt)`: apply velocity to position.

## Notes

- Keep this module small and predictable.
- Do not add rendering/audio concerns here.
- This is a contract layer, not a system layer.
