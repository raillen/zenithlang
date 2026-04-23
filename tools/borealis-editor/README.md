# Borealis Editor

Scaffold inicial do editor Borealis em `Rust + egui`.

## Objetivo desta pasta

Este scaffold existe para validar cedo:

1. a stack do editor;
2. o modelo de app desktop;
3. o fluxo de preview separado;
4. o protocolo inicial de IPC.

## Estado atual

Hoje este editor:

1. abre uma janela desktop simples com `eframe`;
2. mostra o estado do preview;
3. demonstra envelopes JSON do protocolo;
4. prepara a separação entre:
   - app do editor;
   - mensagens;
   - bridge de preview.

Ainda não faz:

1. spawn real do preview Borealis;
2. viewport funcional;
3. scene tree real;
4. inspector real;
5. integração com `project.toml`.

## Arquivos principais

- `src/main.rs`
  - entrada do app.
- `src/app.rs`
  - UI principal do editor.
- `src/messages.rs`
  - envelopes e mensagens do protocolo.
- `src/ipc.rs`
  - bridge inicial de preview/IPC.

## Como rodar

```powershell
cargo run --manifest-path tools/borealis-editor/Cargo.toml
```

## Decisions relacionadas

- `packages/borealis/decisions/011-borealis-editor-stack.md`
- `packages/borealis/decisions/012-borealis-editor-crates.md`
- `packages/borealis/decisions/013-borealis-editor-architecture.md`
- `packages/borealis/decisions/014-borealis-editor-preview-ipc.md`
