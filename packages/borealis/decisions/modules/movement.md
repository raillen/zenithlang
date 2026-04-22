# Borealis Module Decision - Movement

- Status: proposed
- Date: 2026-04-22
- Type: module / gameplay API
- Scope: `borealis.game.movement`

## Summary

Movement holds generic motion helpers that do not belong to a specific controller, vehicle, or scene system.

## Implementation

- keep the API small and readable;
- prefer verbs that describe intent clearly;
- do not mix animation or audio here;
- keep teleport as instant relocation, separate from physical motion.

## Proposed API

- `move(entity, delta)`: moves an entity by a delta.
- `set_position(entity, position)`: sets the absolute position.
- `set_rotation(entity, angle)`: sets the rotation.
- `move_towards(current, target, speed, dt)`: moves gradually toward a target.
- `dash(entity, direction, speed, duration)`: performs a fast short move.
- `push(entity, direction, force)`: applies a directional impulse.
- `teleport(entity, position)`: moves instantly with no travel path.

## Notes

- `push` is the preferred general impulse helper.
- `jump` is controller-specific and should live elsewhere.
