# Borealis Module Decision - Debug

- Status: accepted
- Date: 2026-04-23
- Type: module / diagnostics
- Scope: `borealis.game.debug`

## Summary

Debug contains optional text-first overlays, watches, flags and diagnostics for Borealis scaffolds.

## Implementation

- keep it easy to disable in release builds;
- keep the helpers readable and deterministic;
- prefer text-first helpers in v1 so the module works in stub mode;
- avoid mixing debug visuals into production gameplay modules.

## Accepted API

- `set_enabled(value)`, `is_enabled()`, `clear()`: lifecycle basico do modulo.
- `log(message)`, `log_count()`, `last_log()`: log simples e contadores.
- `set_flag(name, value)`, `toggle(name)`, `flag_enabled(name)`, `flag_count()`: flags nomeadas para overlays/debug views.
- `watch(name, value)`, `unwatch(name)`, `watch_count()`, `watch_value(name)`: watches textuais.
- `watch_text(label, value)`, `watch_entry(name)`: renderizacao textual de watches.
- `grid_text(cell_size, offset_x = 0, offset_y = 0)`: resumo textual de grid.
- `bounds_text(x, y, width, height)`: resumo textual de bounds.
- `hitbox_text(x, y, width, height)`: resumo textual de hitbox.
- `fps_text(fps)`: resumo textual de FPS.
- `overlay_summary()`: resumo agregado do estado debug atual.

## Notes

- debug continua opcional e barato de desligar.
- os helpers atuais sao suficientes para fixtures, scaffolds e futuras overlays do editor.
