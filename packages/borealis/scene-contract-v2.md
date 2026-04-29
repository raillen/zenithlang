# Borealis Scene Contract v2

Scene v2 is the shared contract between:

- `packages/borealis`;
- the SDK metadata;
- Borealis Studio;
- viewport preview;
- Play mode;
- build output.

The editor may add UI state around the scene, but it must not invent a separate
runtime model.

## Required Top-Level Fields

```text
version: 2
name: text
document_id: text
environment: object
render: object
audio: object
entities: list
```

## Environment

`environment` stores world-level settings:

- `skybox`
- `ambient`
- `fog`
- `weather`

These keys map to `sceneSettings` in `borealis.editor.json`.

## Render

`render` stores render pipeline settings:

- `quality`
- `postfx`

Post effects belong here because they affect the final frame, not a single
entity.

## Audio

`audio` stores scene-level mix settings:

- `master`
- `music`
- `sfx`
- `spatial`

Entity audio sources still live in `entities[*].components`.

## Entity Component Shape

Every component has:

```text
kind: text
asset: optional text
script: optional text
profile: optional text
properties: object
```

`kind` must exist in `borealis.editor.json/components`.

## Current Runtime Status

Scene v2 is now the package fixture format. Some settings are still
state-backed or editor-backed until the runtime grows full viewport and Play mode
parity.

The maturity table in `architecture-3d-summary.md` says which modules are
runtime-backed, state-backed, stub, or design-only.
