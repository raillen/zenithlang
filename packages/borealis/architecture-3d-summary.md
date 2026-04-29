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

## Maturity Table

| Module | Maturity | Viewport / Play mode meaning |
|---|---|---|
| `render3d` | runtime-backed | Raylib-backed frame mode, grid, cube, model, billboard |
| `camera3d` | runtime-backed | camera position, target and FOV affect 3D drawing |
| `assets3d` | runtime-backed | models and textures can own native Raylib handles |
| `settings.video` | state-backed | stores render/display choices; full backend application is pending |
| `postfx` | state-backed | scene/editor contract exists; shader-backed effects are pending |
| `world3d` | state-backed | skybox/world data exists; full rendering is pending |
| `world3d.atmosphere` | state-backed | fog/time/wind data exists; viewport fog shader is pending |
| `world3d.weather` | state-backed | weather zones and presets exist; particles/audio are pending |
| `audio3d` | state-backed | listener/emitter state exists; spatial backend integration is pending |
| `physics3d` | stub | gameplay contracts exist; full physics backend is pending |
| `animation3d` | stub | animator state exists; model animation backend is pending |
| `controllers3d` | stub | controller state exists; full movement integration is pending |
| `ai3d` | stub | navigation/perception contracts exist; navmesh/runtime AI is pending |
| `ui3d` | stub | world-space UI state exists; projection/render integration is pending |
| `procedural3d` | stub | generation contracts exist; mesh/terrain runtime output is pending |
| direct `draw_mesh` | design-only | keep using `draw_model` or primitives until native mesh drawing lands |

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
- scene v2 now carries `environment`, `render`, and `audio` settings so Studio,
  viewport, Play mode and build output share one document shape while runtime
  support grows by maturity level.
