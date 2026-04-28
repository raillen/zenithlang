# Borealis Studio Checklist v1

> Checklist operacional do roadmap `borealis-studio-roadmap-v1.md`.
> Status: ativo.
> Atualizado: 2026-04-27.

## Regra de fechamento

Marque `[x]` apenas quando houver evidencia clara: codigo, build, validacao manual, screenshot ou documento atualizado.

## ST0 - Baseline e preservacao

- [x] Criar branch de preservacao `codex/borealis-studio-ui-backup-pre-unity`.
- [x] Criar snapshot local em `tools/borealis-studio/_backup/ui-pre-unity-shell/`.
- [x] Preservar `tools/borealis-studio/ui-design.md`.
- [x] Manter backend Tauri e contrato de preview existentes.

## ST1 - Shell Unity-like

- [x] Criar header global com marca Borealis Studio.
- [x] Centralizar Play/Pause/Step no header.
- [x] Criar tool rail lateral com select, move, rotate, scale, rect e hand.
- [x] Trocar shell escuro antigo por shell claro Unity-like.
- [x] Criar Scene/Game no centro.
- [x] Criar Inspector lateral direito.
- [x] Criar Status Bar fixa.
- [x] Corrigir geometria do dock para a Scene ocupar o centro corretamente.
- [x] Remover acoes de conta, Collab e Share do header.
- [x] Compactar controles visuais da viewport para reduzir ruido.
- [x] Converter controles `Pivot/Center`, `Global/Local`, `2D/3D` e `Gizmos` em dropdowns funcionais.

## ST2 - Docking e layout persistente

- [x] Criar modelo serializavel de dock.
- [x] Criar regioes `leftDock`, `rightDock` e `bottomDock`.
- [x] Permitir resize de Hierarchy, Inspector e bottom dock.
- [x] Permitir mover tabs entre docks suportados.
- [x] Persistir layout por projeto em `localStorage`.
- [x] Criar preset padrao por tamanho de tela.
- [ ] Criar presets de layout no menu `Layout`.
- [ ] Persistir layout no backend Tauri.
- [ ] Suportar reorder de tabs dentro da mesma regiao.

## ST3 - Fluxo de projeto

- [x] Tela inicial com abrir projeto.
- [x] Tela inicial com criar projeto.
- [x] Fallback browser com dados mockados.
- [x] SDK/runtime documentado no README.
- [x] Mostrar projetos recentes.
- [x] Abrir projeto recente com um clique.
- [ ] Dialog nativo para escolher arquivo/pasta.
- [ ] Persistir projeto recente pelo backend Tauri quando em desktop.

## ST4 - Hierarchy e edicao de cena

- [x] Renomear Navigator visual para Hierarchy.
- [x] Exibir parent/child real por `SceneEntity.parent`.
- [x] Criar pseudo-root da cena.
- [x] Selecionar root da cena abre Scene Settings no Inspector.
- [x] Sincronizar selecao com Inspector e Status Bar.
- [x] Reparent por drag-and-drop na Hierarchy.
- [x] Garantir que entidades com `parent` vazio, nulo ou invalido aparecam na raiz da Hierarchy.
- [x] Viewport 2D aplica ferramenta Move no Transform da entidade.
- [x] Viewport 2D aplica ferramenta Rotate em `rotationZ`.
- [x] Viewport 2D aplica ferramenta Scale em `scaleX` e `scaleY`.
- [x] Viewport 2D permite pan com Hand, inclusive quando o arrasto inicia sobre uma entidade.
- [x] Viewport 2D permite selecao por retangulo.
- [x] Viewport 2D calcula posicao de drop de asset a partir do cursor.
- [ ] Reorder entre siblings.
- [ ] Delete de entidade selecionada.
- [ ] Duplicate de entidade selecionada.
- [ ] Multi-select.
- [ ] Copy/Paste/Cut.
- [ ] Gizmo 2D com handles visuais reais por eixo/rotacao/escala.
- [ ] Undo/redo para mutacoes feitas pela viewport.

## ST5 - Inspector e componentes

- [x] Cabecalho Unity-like do objeto selecionado.
- [x] Linha Tag/Layer.
- [x] Grupo Transform.
- [x] Component cards guiados por manifesto/catalogo.
- [x] Botao Add Component no fim do Inspector.
- [x] Scene Settings quando root da cena esta selecionado.
- [x] Add Component com menu categorizado.
- [x] Corrigir scroll interno do Inspector.
- [x] Adicionar catalogo fallback de Scene Settings quando o manifesto nao fornece schema.
- [ ] Remover componente pelo backend.
- [ ] Validar singleton/requires de componente.
- [ ] Inspector para multi-select com campos comuns.

## ST6 - Project, assets e Console

- [x] Trocar Assets por Project.
- [x] Consolidar `Project | Console | Animation`.
- [x] Remover Problems como tab separada.
- [x] Mostrar warnings/errors em Console e Status Bar.
- [x] Manter drag-and-drop de asset para Scene.
- [x] Adicionar asset publico `triangle.obj` para fallback browser do viewport.
- [x] Corrigir grid/zoom do Project para evitar sobreposicao de assets.
- [x] Refinar visual das tabs `Project | Console | Animation`.
- [ ] Asset scan real no backend.
- [ ] Import asset por dialog/drop externo.
- [ ] Metadata real por asset.
- [ ] Thumbnails reais.
- [ ] Animation tab funcional.

## ST7 - Game view e preview

- [x] Criar aba `Game`.
- [x] Mostrar estado do preview na aba `Game`.
- [x] Play no header muda para aba `Game`.
- [x] Corrigir controles internos da viewport para nao ficarem cortados em largura estreita.
- [x] Trocar controle de tamanho do grid para campo numerico.
- [x] Adicionar preferencias persistidas de cor, opacidade e exibicao do grid.
- [x] Tornar janela de configuracoes centralizada, arrastavel e com secoes de Viewport, Grid, Appearance e Keybinds.
- [ ] Embutir frames reais do runner na aba `Game`.
- [ ] Mostrar stream de logs do preview sem depender de reload.
- [ ] Hot-reload de cena no preview.
- [ ] Isolar mudancas feitas em Play Mode.

## ST8 - Backend como autoridade de editor

- [x] Adicionar command Tauri para `reparent_entity`.
- [x] Adicionar command Tauri para `get_entity_tree`.
- [ ] Mover add entity para command Tauri.
- [ ] Mover update transform para command Tauri.
- [ ] Mover update component para command Tauri.
- [ ] Criar selection state no backend.
- [ ] Criar undo/redo no backend.
- [ ] Criar clipboard no backend.

## ST9 - Standalone e release

- [x] Documentar layout recomendado de release com `runtime/sdk`.
- [x] `npm run sdk:assemble` documentado.
- [ ] Validar Studio fora do checkout `zenith-lang-v2`.
- [ ] Smoke test de release standalone.
- [ ] Empacotar SDK no instalador.
- [ ] CI para build do Studio.

## Gates executados neste corte

- [x] `npm run build` em `tools/borealis-studio`.
- [x] Browser abre `http://127.0.0.1:1420/`.
- [x] Browser abre projeto mock Borealis.
- [x] Browser valida Scene central sem layout quebrado.
- [x] Browser valida root da cena no Inspector.
- [x] Browser valida tab drag-and-drop entre docks.
- [x] Browser valida persistencia local de layout.
- [x] Browser valida projetos recentes na Home.
- [x] Browser valida Add Component categorizado.
- [x] Browser valida console sem warnings novos apos reload limpo.
- [x] `cargo check` depois das proximas mudancas Tauri.
- [x] `npm run build` apos refinamento visual do shell.
- [x] Browser valida header sem Account/Collab/Share.
- [x] Browser valida scroll do Inspector.
- [x] Browser valida zoom do Project sem sobreposicao.
- [x] Browser valida dropdowns funcionais da toolbar da viewport.
- [x] Browser valida painel de view controls contido na viewport.
- [x] Browser valida janela de configuracoes centralizada e arrastavel.
- [x] Browser valida Viewport 2D com Move, Rotate e Scale alterando transform visual.
- [x] Browser valida Viewport 2D com Hand pan e Rect selection.
