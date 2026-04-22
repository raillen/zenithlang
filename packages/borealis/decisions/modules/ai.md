# Borealis Module Decision - AI

- Status: proposed
- Date: 2026-04-22
- Type: module / gameplay AI
- Scope: `borealis.game.ai`

## Summary

AI holds basic decision-making helpers, behavior control, and perception utilities.

## Implementation

- keep names explicit and easy to understand;
- prefer behavior words over abstract AI jargon;
- split perception from decision-making when possible.

## Proposed API

- `look_at(entity, target)`: makes an entity face a target.
- `distance(a, b) -> float`: returns distance.
- `move_to_point(entity, point)`: moves toward a point.
- `face_target(entity, target)`: turns toward a target.
- `chase_target(entity, target)`: pursues a target.
- `run_away(entity, target)`: moves away from a target.
- `evade(entity, target)`: escapes with more evasive behavior.
- `patrol(entity, points[])`: follows a patrol route.
- `guard(entity, area)`: guards an area.
- `investigate(entity, point)`: moves to inspect a point.
- `return_to_point(entity, point)`: returns to a home point.
- `idle(entity)`: keeps the entity idle.
- `set_target(entity, target)`: sets the current target.
- `clear_target(entity)`: clears the current target.
- `can_see(entity, target) -> bool`: checks visibility.
- `has_line_of_sight(entity, target) -> bool`: checks clear sight.
- `nearest_target(entity, radius) -> Entity`: finds the nearest target.
- `vision_cone(entity, angle, distance) -> Entity[]`: finds entities inside view.
- `hearing_sensor(entity, radius) -> Event[]`: finds nearby events.
- `fsm_create() -> FSM`: creates a finite state machine.
- `fsm_set_state(fsm, state)`: changes FSM state.
- `bt_create() -> BT`: creates a behavior tree.
- `ai_update(ai, dt)`: updates AI logic.

## Notes

- `wander` is replaced by `move_randomly` in later naming work.
- `flee` is replaced by `evade`.
