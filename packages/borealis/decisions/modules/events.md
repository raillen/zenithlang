# Borealis Module Decision - Events

- Status: proposed
- Date: 2026-04-22
- Type: module / messaging
- Scope: `borealis.game.events`

## Summary

Events handle simple event dispatch and gameplay notifications.

## Implementation

- keep it lightweight;
- keep dispatch easy to understand;
- do not turn it into a complex enterprise event bus.

## Proposed API

- `event_emit(name, payload)`: emits an event.
- `event_on(name, handler)`: registers a handler.
- `event_off(name, handler)`: removes a handler.
- `event_once(name, handler)`: registers a one-time handler.
- `event_clear(name)`: clears handlers for a name.
- `event_dispatch(name, payload)`: dispatches an event.

## Notes

- events should help decouple gameplay and UI.
