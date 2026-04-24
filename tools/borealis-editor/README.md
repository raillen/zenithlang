# Borealis Editor

Editor inicial do Borealis em `Rust + egui`.

## Objetivo desta pasta

Este scaffold existe para validar cedo:

1. a stack do editor;
2. o modelo de app desktop;
3. o fluxo de preview separado;
4. o protocolo inicial de IPC.

## Estado atual

Hoje este editor ja esta pronto para um uso inicial de cena:

1. abre uma janela desktop simples com `eframe`;
2. mostra o estado do preview;
3. demonstra envelopes JSON do protocolo;
4. le um `zenith.ztproj` e mostra um resumo do projeto Borealis;
5. envia `open_project` de forma explicita para o preview;
6. monta uma arvore inicial com cenas candidatas, exemplos e assets;
7. permite apontar manualmente para um preview compilado ou para `zt.exe`;
8. mostra um inspector basico para o item selecionado na arvore;
9. carrega `packages/borealis/scenes/sample.scene.json` como contrato inicial de cena;
10. o preview runner le `open_scene` e emite diagnostico com contagem de entidades;
11. organiza a UI em docks no estilo editor de jogo:
   - toolbar no topo;
   - Hierarchy/Project a esquerda;
   - Scene View no centro;
   - Inspector a direita;
   - Console embaixo.
12. usa IDs estaveis para os docks principais, facilitando testes futuros;
13. carrega a cena inicial automaticamente e mostra a entidade selecionada no Inspector;
14. desenha uma pre-visualizacao simples da cena com grid, entidades e gizmos no Scene View;
15. permite selecionar entidades pelo `Scene View`, sincronizando com o Inspector;
16. permite arrastar entidades no `Scene View`, atualizando o `transform` carregado;
17. permite editar `name`, `layer`, `parent`, `tags` e `transform` 2D/3D pelo Inspector;
18. salva de volta o `scene.json` preservando o contrato atual e os componentes existentes;
19. permite editar `name` e `document_id` da cena, com status `dirty/saved`;
20. oferece `Save Scene`, `Save + Sync` e `Reload Scene` no fluxo do Inspector;
21. cria e remove entidades direto da cena carregada, com persistencia no `scene.json`;
22. inicia o preview e ja abre o projeto e a cena atual sem exigir passos manuais extras;
23. sincroniza a cena atual com o preview pelo toolbar ou automaticamente ao salvar;
24. bloqueia troca e recarga de cena quando ha alteracoes locais nao salvas;
25. aceita preview compilado ou `zt.exe` como runner do preview;
26. tem smoke test do estado inicial do editor e do contrato de cena;
27. adiciona uma aba `Scene 3D` com grid isometrico, eixos XYZ, criacao de `cube3d`/`camera3d` e drop de modelos;
28. reconhece assets `.obj` no Project e inclui uma cena exemplo 3D em `sample_3d.scene.json`;
29. prepara a separacao entre:
   - app do editor;
   - mensagens;
   - bridge de preview;
   - leitura local de projeto.

## O que ja da para testar agora

1. abrir o editor;
2. carregar `packages/borealis/scenes/sample.scene.json`;
3. selecionar e arrastar entidades no `Scene View`;
4. abrir `Scene 3D`, criar objetos 3D e soltar modelos do Project na viewport;
5. editar `name`, `layer`, `tags`, `parent` e `transform` no `Inspector`;
6. criar e remover entidades;
7. salvar a cena e reabrir;
8. iniciar o preview em modo mock;
9. apontar para o preview real compilado ou para `zt.exe`;
10. sincronizar a cena atual com o preview.

## O que ainda falta depois do uso inicial

1. manipuladores por eixo com constraint real de X/Y/Z;
2. componentes editaveis por tipo, nao so por lista;
3. hierarquia de cena com parent/child visual mais rica;
4. preview visual realmente embutido;
5. fluxo de prefabs e materiais.

## Arquivos principais

- `src/main.rs`
  - entrada do app.
- `src/app.rs`
  - UI principal do editor, com layout em docks.
- `src/messages.rs`
  - envelopes e mensagens do protocolo.
- `src/ipc.rs`
  - bridge inicial de preview/IPC.
- `src/project.rs`
  - leitura leve de `zenith.ztproj` e resumo do pacote.
- `src/scene_document.rs`
  - leitura minima de cenas JSON e exemplos Zenith com entidades.
- `packages/borealis/scenes/sample.scene.json`
  - fixture inicial de cena compartilhada com o package Borealis.
- `packages/borealis/scenes/sample_3d.scene.json`
  - fixture inicial para edicao visual 3D.

## Como rodar

```powershell
cargo run --manifest-path tools/borealis-editor/Cargo.toml
```

## Primeiro uso recomendado

1. abra o editor;
2. confirme `packages/borealis/zenith.ztproj` no campo de projeto;
3. clique `Start`;
4. se quiser usar o preview real, aponte `Preview binario ou zt.exe` para:
   - `tools/borealis-editor/preview/build/borealis-preview.exe`, ou
   - `zt.exe`.
5. edite a cena, use `Save` ou `Save Scene`, e deixe `Auto Sync` ligado para empurrar a cena ao preview.

## Como testar

```powershell
$env:CARGO_INCREMENTAL='0'; cargo test --manifest-path tools/borealis-editor/Cargo.toml
cargo check --manifest-path tools/borealis-editor/Cargo.toml
```

## Smoke do preview

```powershell
.\zt.exe run tools/borealis-editor/preview/zenith.ztproj
'{"protocol":1,"seq":1,"channel":"command","kind":"open_scene","payload":{"path":"scenes/sample.scene.json"}}' | .\tools\borealis-editor\preview\build\borealis-preview.exe
```

Resposta esperada:

```json
{"protocol":1,"seq":2,"channel":"event","kind":"diagnostic","payload":{"level":"info","message":"open_scene path=packages/borealis/scenes/sample.scene.json entities=3 loaded=true"}}
```

## Decisions relacionadas

- `packages/borealis/decisions/011-borealis-editor-stack.md`
- `packages/borealis/decisions/012-borealis-editor-crates.md`
- `packages/borealis/decisions/013-borealis-editor-architecture.md`
- `packages/borealis/decisions/014-borealis-editor-preview-ipc.md`
