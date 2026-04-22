# Decision 007 - Borealis Flow

- Status: proposed
- Date: 2026-04-22
- Type: editor direction / visual scripting / low-code
- Scope: future Borealis editor logic layer

## Summary

`Borealis Flow` is the proposed visual logic layer for the future Borealis editor.

It is not meant to replace programming entirely. It is meant to provide a low-code path for common gameplay, UI, and sequence logic while remaining compatible with Zenith code.

## Decision

Adopt this direction for `Borealis Flow`:

1. `Borealis Flow` is a visual scripting / low-code layer.
2. It sits on top of Borealis modules instead of replacing them.
3. It should call Borealis APIs rather than inventing a separate runtime model.
4. It should coexist with Zenith code.
5. The first target is common gameplay and UI logic, not full engine replacement.

## Rationale

This gives Borealis:

- beginner-friendly prototyping;
- faster iteration for common gameplay cases;
- a path similar to GameMaker/Construct ease;
- growth into code when complexity rises.

It avoids creating a second hidden language with separate rules.

## Graph Families

The preferred early graph families are:

1. Event Graph
   - reacts to collisions, clicks, input, triggers, and gameplay events
2. State Graph
   - controls state transitions such as idle, move, chase, attack
3. UI Flow Graph
   - controls menus, popups, visibility, widget logic
4. Sequence Graph
   - controls cutscenes, waits, ordered actions, and simple scripted flows

## Node Categories

Preferred early node categories:

1. Event
2. Condition
3. Action
4. Value
5. Wait
6. Branch
7. Loop
8. Call
9. State
10. Transition

## Runtime Integration

`Borealis Flow` should integrate through existing modules:

- movement nodes call `game.movement`
- audio nodes call `game.audio`
- animation nodes call `game.animation`
- event nodes call `game.events`
- save nodes call `game.save`
- scene nodes call `game.scene`

## Code Integration

`Borealis Flow` should support three levels:

1. visual-only logic;
2. visual graphs that call Zenith functions;
3. code-only logic for advanced cases.

## Non-Canonical Forms

1. treating Flow as a total replacement for Zenith;
2. creating a separate runtime model just for visual logic;
3. trying to ship full no-code coverage before the runtime/editor model is stable.

## Out Of Scope

This decision does not finalize:

1. the visual editor UI;
2. the serialization format of graph assets;
3. advanced graph families such as shader graphs or complex AI graphs.
