# Borealis Module Decision - Events

- Status: accepted
- Date: 2026-04-23
- Type: module / messaging
- Scope: `borealis.game.events`

## Summary

Events handle a simple event queue plus named listener deliveries for gameplay, UI and scaffold communication.

## Implementation

- keep one simple global queue for emitted events;
- support named listeners with `on`, `off` e `once`;
- route listener deliveries into deterministic per-listener queues;
- keep payloads as `text` in v1;
- avoid turning the module into a complex event bus.

## Accepted API

- `emit(name, payload = "")`: enfileira um evento.
- `queue(name, payload = "")`: alias explicito para fila.
- `dispatch(name, payload = "")`: emite e roteia o evento para listeners registrados.
- `on(name, listener)`: registra um listener persistente.
- `once(name, listener)`: registra um listener de uso unico.
- `off(name, listener)`: desativa um listener.
- `clear(name = "")`: limpa listeners de um nome especifico ou todos.
- `peek()`, `poll()`, `dispatch_next()`: introspeccao/drain da fila global.
- `listener_peek(listener)`, `listener_poll(listener)`, `clear_listener(listener)`: introspeccao/drain por listener.
- `is_registered(name, listener)`, `listener_count(name = "")`: estado dos listeners.
- `clear_pending(name = "")`, `has_pending()`, `pending_count()`: estado da fila global.
- `listener_has_pending(listener)`, `listener_pending_count(listener)`: estado da fila por listener.
- `count_named(name)`, `count()`, `last_name()`, `last_payload()`, `reset()`: helpers de telemetria e reset.

## Notes

- `dispatch(...)` retorna quantas entregas foram roteadas para listeners.
- `once(...)` remove o listener depois da primeira entrega.
- o foco do modulo continua sendo desacoplamento simples entre gameplay, UI e scaffolds.
