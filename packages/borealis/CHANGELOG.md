# Borealis Changelog

## 0.1.0 - 2026-04-22

Primeira baseline publica do Borealis como package 2D da Zenith.

Inclui:

- duas camadas publicas: `borealis.game` e `borealis.engine`
- loop base de janela/frame
- draw 2D inicial: `line`, `rect`, `rect_outline`, `circle`, `circle_outline`, `text`
- input por frame: `key_down`, `key_pressed`, `key_released`
- `scene` e `entities` v1
- subset inicial de ECS em `borealis.engine.ecs`
- scaffolds modulares 2D e trilha 3D experimental/documental
- hook desktop no runtime C com adapter Raylib inicial por carga dinamica
- fallback seguro para stub quando backend desktop nao estiver disponivel

Tambem inclui ajustes desta rodada:

- `save.slot_read(...)` agora retorna `none` para slot apagado
- `storage.read(...)` agora retorna `none` para chave removida
- `save` e `storage` passaram a materializar `text` local antes de `map_set`, preparando melhor compatibilidade com o emitter C atual

Observacao:

- esta versao ainda nao fecha todos os cenarios `run-pass` do Borealis por limites conhecidos do backend C
- os limites atuais estao documentados em `packages/borealis/known-limits-v1.md`
