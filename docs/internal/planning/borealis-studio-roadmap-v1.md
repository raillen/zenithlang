# Borealis Studio Roadmap v1

> Roadmap operacional do editor Borealis Studio.
> Status: ativo para a interface Unity-like e fluxo inicial do Studio.
> Atualizado: 2026-04-27.

## Objetivo

Transformar o Borealis Studio em um editor de engine usavel, com fluxo inspirado no Unity e contrato fiel ao package `packages/borealis`.

O Studio deve ser uma camada de autoria. O contrato de cena, componentes, assets e preview continua vindo do Borealis/SDK.

## Principios

- A primeira tela deve abrir ou criar projeto, nao vender o produto.
- A interface deve ser densa, clara e previsivel.
- Hierarchy, Scene/Game, Inspector, Project, Console e Status Bar sao paineis centrais.
- O Inspector deve ser schema-driven pelo `borealis.editor.json`.
- O editor nao deve criar um formato paralelo que perca campos da cena.
- O Studio deve funcionar como produto standalone com SDK/runtime empacotado.

## Fases

### ST0 - Baseline e preservacao

Objetivo:

- preservar a interface anterior;
- registrar o `ui-design.md` como contrato visual;
- manter o backend Tauri, preview e manifesto existentes funcionando.

Estado: concluido no corte atual.

### ST1 - Shell Unity-like

Objetivo:

- substituir o shell antigo por header global, tool rail, docks, Scene/Game, Inspector e Status Bar;
- usar visual claro, tecnico e proximo do mock;
- manter build e fallback browser funcionando.

Estado: concluido no corte atual.

### ST2 - Docking e layout persistente

Objetivo:

- permitir tabs entre regioes suportadas;
- permitir resize de Hierarchy, Inspector e Project;
- persistir layout por projeto;
- manter preset padrao coerente com `ui-design.md`.

Estado: concluido no corte atual, sem floating windows.

### ST3 - Fluxo de projeto

Objetivo:

- abrir projeto;
- criar projeto;
- mostrar projetos recentes;
- manter layout por projeto;
- deixar o usuario voltar ao ultimo contexto sem reconfigurar tudo.

Estado: parcial.

### ST4 - Hierarchy e edicao de cena

Objetivo:

- exibir parent/child real;
- selecionar root da cena;
- selecionar entidade;
- reparent por drag-and-drop;
- editar entidades pela viewport 2D com Move, Rotate, Scale, Hand e Rect;
- preparar delete, duplicate, reorder, multi-select, gizmos por eixo e undo/redo.

Estado: parcial. A primeira fatia funcional da viewport 2D ja altera Transform real no frontend e valida pan/selecao no browser.

### ST5 - Inspector e componentes

Objetivo:

- manter Inspector no fluxo Unity-like;
- editar identidade, tag, layer, transform e componentes;
- preservar campos desconhecidos;
- evoluir Add Component para menu categorizado pelo manifesto.

Estado: parcial.

### ST6 - Project, assets e Console

Objetivo:

- consolidar `Project | Console | Animation`;
- manter drag-and-drop de assets para a cena;
- mostrar warnings/errors no Console e Status Bar;
- preparar import, scan e metadata real no backend.

Estado: parcial.

### ST7 - Game view e preview embutido

Objetivo:

- transformar a aba `Game` em preview embutido real;
- refletir Play/Pause/Stop no viewport;
- manter o preview sidecar como processo isolado;
- mostrar erros de runtime no Console.

Estado: aberto.

### ST8 - Backend como autoridade de editor

Objetivo:

- mover mutacoes criticas de cena para commands Tauri;
- adicionar undo/redo;
- adicionar selection state real;
- adicionar clipboard, duplicate, delete e multi-select;
- reduzir mutacao direta de entidades no frontend.

Estado: aberto.

### ST9 - Standalone e release

Objetivo:

- empacotar SDK/runtime com o Studio;
- validar fora do checkout `zenith-lang-v2`;
- manter `BOREALIS_SDK_ROOT` como override de desenvolvimento;
- preparar smoke tests de release.

Estado: parcial.

## Gates

- `npm run build` em `tools/borealis-studio`.
- `cargo check` em `tools/borealis-studio/src-tauri`.
- Validacao visual no browser ou Tauri quando houver mudanca de UI.
- Teste manual minimo: abrir projeto, selecionar entidade, selecionar root da cena, mover tab, abrir Console.

## Proximo foco

1. Consolidar viewport 2D Unity-like com gizmos por eixo, multi-select e undo/redo.
2. Dialog nativo para abrir arquivo/pasta.
3. Preview embutido real na aba `Game`.
4. Presets de layout no menu `Layout`.
5. Asset scan/import real no backend.
