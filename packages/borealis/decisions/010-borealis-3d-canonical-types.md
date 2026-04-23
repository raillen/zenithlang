# Borealis Decision 010 - 3D Canonical Types and Naming

- Status: proposed
- Date: 2026-04-22
- Type: architecture / naming and shared types
- Scope: Borealis 3D modules

## Summary

Borealis 3D uses shared spatial primitives from `borealis.game`.
Module-specific IDs and states stay inside their own modules.

## Canonical shared types

- `game.Vector3`: direction, velocity, rotation delta, generic spatial vector.
- `game.Point3`: point in world space.
- `game.Size3`: width, height, depth.
- `game.Transform3D`: position, rotation, scale.
- `game.Bounds3D`: min/max 3D bounds.
- `game.Ray3`: origin + direction for hits and picks.
- `game.Point2`: pure screen point.
- `game.Size2`: width/height pair.
- `game.Color`: user-facing color type for easy-layer APIs.

## Naming rules

1. Shared geometry types should not be recreated in each 3D module.
2. IDs stay module-owned:
   - `assets3d.MeshId`
   - `render3d.LightId`
   - `physics3d.BodyId`
   - `animation3d.AnimatorId`
3. Shared modules remain shared:
   - `scene`
   - `save`
   - `events`
   - `services`
   - `storage`
   - `database`
   - `input`
   - `settings`
4. 3D convenience helpers should not duplicate the canonical systems:
   - `entities3d` extends `entities`
   - `ai3d` extends `ai`
   - `procedural3d` extends `procedural`

## Notes

- older design notes that mention conceptual `math.*` types should be read as the canonical `game.*` types above.
- this decision exists to keep the documentation and the future runtime scaffolds aligned.
