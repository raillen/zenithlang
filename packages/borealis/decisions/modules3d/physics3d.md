# Borealis Module Decision - Physics3D

- Status: proposed
- Date: 2026-04-22
- Type: module / simplified 3D physics API
- Scope: `borealis.game.physics3d`

## Summary

Physics3D provides simple and predictable collision and movement for classic 3D games.
It focuses on useful gameplay behavior instead of full simulation complexity.

## Implementation

- keep a beginner-friendly API with direct functions;
- support common colliders (box, sphere, capsule);
- provide ray and overlap queries for gameplay logic;
- keep effects simple and scriptable (impulse, explosion, knockback).

## Proposed API

### Body lifecycle and movement

- `body3d_create(kind: physics3d.BodyKind, transform: game.Transform3D) -> result<physics3d.BodyId, core.Error>`: creates a physics body.
- `body3d_destroy(id: physics3d.BodyId) -> result<void, core.Error>`: removes a physics body.
- `body3d_exists(id: physics3d.BodyId) -> bool`: checks if body exists.
- `body3d_set_velocity(id: physics3d.BodyId, velocity: game.Vector3) -> result<void, core.Error>`: sets velocity.
- `body3d_get_velocity(id: physics3d.BodyId) -> result<game.Vector3, core.Error>`: reads velocity.
- `body3d_add_force(id: physics3d.BodyId, force: game.Vector3) -> result<void, core.Error>`: adds continuous force.
- `body3d_apply_impulse(id: physics3d.BodyId, impulse: game.Vector3) -> result<void, core.Error>`: applies instant impulse.
- `body3d_move_and_slide(id: physics3d.BodyId, input: game.Vector3, dt: float) -> result<void, core.Error>`: character-style movement with sliding.
- `body3d_on_ground(id: physics3d.BodyId) -> bool`: checks if body is grounded.
- `body3d_stop(id: physics3d.BodyId) -> result<void, core.Error>`: clears linear and angular velocity.
- `body3d_teleport(id: physics3d.BodyId, position: game.Vector3) -> result<void, core.Error>`: instant move without path simulation.
- `body3d_snap_to_ground(id: physics3d.BodyId, max_distance: float) -> result<bool, core.Error>`: snaps body to nearest ground.

### Body properties

- `body3d_set_enabled(id: physics3d.BodyId, enabled: bool) -> result<void, core.Error>`: enables/disables simulation.
- `body3d_is_enabled(id: physics3d.BodyId) -> bool`: checks if body is enabled.
- `body3d_set_kinematic(id: physics3d.BodyId, enabled: bool) -> result<void, core.Error>`: toggles kinematic mode.
- `body3d_is_kinematic(id: physics3d.BodyId) -> bool`: checks kinematic mode.
- `body3d_set_mass(id: physics3d.BodyId, mass: float) -> result<void, core.Error>`: sets body mass.
- `body3d_get_mass(id: physics3d.BodyId) -> result<float, core.Error>`: gets body mass.
- `body3d_set_friction(id: physics3d.BodyId, value: float) -> result<void, core.Error>`: sets friction.
- `body3d_get_friction(id: physics3d.BodyId) -> result<float, core.Error>`: gets friction.
- `body3d_set_bounce(id: physics3d.BodyId, value: float) -> result<void, core.Error>`: sets bounce/restitution.
- `body3d_get_bounce(id: physics3d.BodyId) -> result<float, core.Error>`: gets bounce/restitution.
- `body3d_set_gravity_scale(id: physics3d.BodyId, scale: float) -> result<void, core.Error>`: adjusts gravity effect.
- `body3d_get_gravity_scale(id: physics3d.BodyId) -> result<float, core.Error>`: reads gravity scale.
- `body3d_set_linear_damping(id: physics3d.BodyId, value: float) -> result<void, core.Error>`: applies damping over time.
- `body3d_get_linear_damping(id: physics3d.BodyId) -> result<float, core.Error>`: reads damping value.

### Colliders

- `collider_set_box(id: physics3d.BodyId, size: game.Vector3) -> result<void, core.Error>`: sets AABB collider.
- `collider_set_sphere(id: physics3d.BodyId, radius: float) -> result<void, core.Error>`: sets sphere collider.
- `collider_set_capsule(id: physics3d.BodyId, radius: float, height: float) -> result<void, core.Error>`: sets capsule collider.

### Queries and contacts

- `raycast(origin: game.Vector3, direction: game.Vector3, distance: float, mask: int) -> optional<game.Ray3Hit>`: single-hit ray query.
- `raycast_all(origin: game.Vector3, direction: game.Vector3, distance: float, mask: int) -> list<game.Ray3Hit>`: multi-hit ray query.
- `shape_overlap(shape: physics3d.QueryShape, transform: game.Transform3D, mask: int) -> list<physics3d.BodyId>`: overlap query by shape.
- `overlap_box(center: game.Vector3, size: game.Vector3, mask: int) -> list<physics3d.BodyId>`: box overlap query.
- `overlap_sphere(center: game.Vector3, radius: float, mask: int) -> list<physics3d.BodyId>`: sphere overlap query.
- `body3d_get_contacts(id: physics3d.BodyId) -> list<physics3d.Contact>`: returns active contacts.

### Simulation step

- `physics3d_step(dt: float) -> result<void, core.Error>`: updates simulation.
- `physics3d_set_fixed_timestep(dt: float) -> result<void, core.Error>`: sets fixed simulation step.
- `physics3d_get_fixed_timestep() -> float`: gets fixed simulation step.

### Gameplay effects

- `physics3d_explosion(center: game.Vector3, radius: float, force: float, mask: int) -> result<void, core.Error>`: radial impulse effect.
- `body3d_apply_knockback(id: physics3d.BodyId, origin: game.Vector3, force: float) -> result<void, core.Error>`: pushes body away from an origin.

## Notes

- API is focused on gameplay usefulness and easy onboarding.
- deeper simulation features can remain in `borealis.engine` for advanced use.

