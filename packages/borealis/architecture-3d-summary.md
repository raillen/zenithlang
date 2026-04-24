# Borealis 3D Architecture Summary

## Goal

One compact reference for the Borealis 3D design track:
module roles, canonical types, and which modules stay shared.

## Canonical Types

| Type | Use |
|---|---|
| `game.Vector3` | direction, velocity, generic vector math |
| `game.Point3` | world-space point |
| `game.Size3` | width, height, depth |
| `game.Transform3D` | position, rotation, scale |
| `game.Bounds3D` | min/max bounds |
| `game.Ray3` | raycasts and picking |
| `game.Point2` | screen-space point |
| `game.Size2` | width/height pair |
| `game.Color` | easy-layer color |

## Summary Table

| Module | Purpose | Key types | Notes |
|---|---|---|---|
| `core3d` | runtime loop, states, frame flow | `Runtime3D`, `StateStack` | architectural/runtime helper |
| `render3d` | drawing, lights, billboards, quality profile | `LightId`, `PointLightState` | consumes `assets3d` IDs |
| `camera3d` | camera control + presets | `Camera3D`, `CameraMode` | `camera.presets` is the easy path |
| `physics3d` | bodies, colliders, raycasts, overlap | `BodyId`, `Contact` | simplified gameplay-first physics |
| `world3d` | terrain, chunks, sky, bounds, grid | `World3D`, `ChunkCoord`, `LayerHandle` | consumes `procedural3d` and `assets3d` |
| `world3d.atmosphere` | fog, time-of-day, wind, ambient profile | `FogState`, `WindState` | world-space only |
| `world3d.weather` | rain, snow, storm, lightning, zones | `WeatherZoneId`, `WeatherKind` | lightweight, not realistic simulation |
| `assets3d` | meshes, models, materials, textures, cubemaps, clips | `MeshId`, `ModelHandle`, `MaterialHandle` | owns cache and memory budget |
| `animation3d` | clip playback, blend, events | `AnimatorId`, `AnimEvent` | `assets3d` owns clip loading |
| `audio3d` | spatial sound, listener, emitters | `Sound3dId`, `InstanceId` | gameplay-focused positional audio |
| `entities3d` | helper layer for 3D entities | `Entity3DDebugInfo` | extends `entities`, does not replace it |
| `controllers3d` | ready-made 3D controllers | `Controller3DHandle` | `vehicles` stays separate |
| `ai3d` | navigation, perception, steering, combat | `Ai3dId`, `Path3D` | extends `ai`, does not replace it |
| `ui3d` | world-space UI and projection bridge | `Ui3dId`, `AnchorMode` | HUD remains in `ui` |
| `procedural3d` | terrain, scatter, routes, recipes | `Proc3DHandle`, `HeightField` | extends `procedural` |
| `postfx` | screen-space visual finishing | `PostFxState` | includes `fxaa`; excludes `vsync` and `msaa` |
| `settings.video` | render/display settings | `VideoSettings`, `AntiAliasingMode` | owns `vsync`, `msaa`, `render_scale` |
| `debug3d` | gizmos, bounds, paths, chunk overlays | `Debug3dOverlayState` | 3D-specific diagnostics |

## Shared Modules Stay Shared

These are not duplicated for 3D:

- `scene`
- `save`
- `events`
- `services`
- `storage`
- `database`
- `input`
- `settings`

## Notes

- this file is still architectural first, but the first runtime-backed 3D slice now exists.
- implemented slice: `render3d_begin/end`, `draw_cube`, `draw_grid`, `draw_model`, `draw_billboard`, Raylib model loading and native texture handles.
- still pending: direct `draw_mesh`, materials, generated meshes, lighting shaders, animation and full 3D scene/editor integration.
