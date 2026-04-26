# Borealis Studio — Plano de Implementacao Completo

## Indice

1. [Diagnostico do estado atual](#1-diagnostico-do-estado-atual)
2. [Principios e regras](#2-principios-e-regras)
3. [Arquitetura alvo](#3-arquitetura-alvo)
4. [Roadmap de fases](#4-roadmap-de-fases)
5. [Fase 0 — Infraestrutura](#5-fase-0--infraestrutura)
6. [Fase 1 — Modularizacao do Backend Rust](#6-fase-1--modularizacao-do-backend-rust)
7. [Fase 2 — Modularizacao do Frontend React](#7-fase-2--modularizacao-do-frontend-react)
8. [Fase 3 — Motor de cena e entidades no backend](#8-fase-3--motor-de-cena-e-entidades-no-backend)
9. [Fase 4 — Sistema de componentes real](#9-fase-4--sistema-de-componentes-real)
10. [Fase 5 — Asset pipeline](#10-fase-5--asset-pipeline)
11. [Fase 6 — Undo/Redo e Selection](#11-fase-6--undoredo-e-selection)
12. [Fase 7 — Viewport e rendering](#12-fase-7--viewport-e-rendering)
13. [Fase 8 — Build e preview](#13-fase-8--build-e-preview)
14. [Fase 9 — Editors especializados](#14-fase-9--editors-especializados)
15. [Fase 10 — Polish e release](#15-fase-10--polish-e-release)
16. [Checklist master](#16-checklist-master)
17. [Referencia Unity](#17-referencia-unity)

---

## 1. Diagnostico do estado atual

### Problemas criticos

| Problema | Impacto | Arquivo |
|---|---|---|
| **App.tsx monolito** (3644 linhas, 120KB) | Impossivel manter, testar ou extender | `src/App.tsx` |
| **styles.css monolito** (2038 linhas, 36KB) | Conflitos de classe, sem escopo | `src/styles.css` |
| **main.rs monolito** (1664 linhas, 52KB) | Sem separacao de responsabilidade | `src-tauri/src/main.rs` |
| **Logica de negocio no frontend** | Scene manipulation, serialization, entity management, undo-candidate operations vivem no React | `App.tsx`, `sceneSerialization.ts` |
| **Estado global via useState** | 20+ useState no componente raiz, prop drilling por 15+ niveis | `App.tsx:130-220` |
| **Sem undo/redo** | Qualquer edicao e irreversivel | - |
| **Sem tipagem forte de componentes** | Componentes sao `Record<string, unknown>` | `types.ts`, `borealisCatalog.ts` |
| **3D viewport mostra placeholders** | Cubos e esferas, nao assets reais | `Viewport3DEntity` |
| **Hierarquia flat** | Parent-child nao e visual no Navigator | `NavigatorPanel` |
| **Sem delete/copy/multi-select** | Operacoes basicas de editor ausentes | - |
| **Script editor sem syntax highlight** | Textarea puro | `CodePanel` |

### O que funciona e deve ser preservado

- Ponte Tauri (IPC para preview, file I/O)
- Formato de cena JSON v2
- Catalogo de componentes com schema
- Viewport 3D base (Three.js + react-three-fiber + drei)
- Sistema de templates de projeto
- Editor manifest dynamico
- Temas e keybinds

---

## 2. Principios e regras

### Clean Code

1. **Single Responsibility** — cada modulo/arquivo faz uma coisa
2. **Max 300 linhas por arquivo** (soft limit) — ideal 150-250
3. **Max 50 linhas por funcao** (hard limit)
4. **Nenhum prop drilling alem de 2 niveis** — usar store ou context
5. **Tipos explicitos** — zero `any`, zero `unknown` sem type guard
6. **Nomes descritivos** — sem abreviacoes (exceto padrao: id, ui, etc)

### Separacao Backend/Frontend

```
BACKEND (Rust/Tauri) — "Source of Truth"
  - Estado da cena (entidades, componentes, transforms)
  - Manipulacao da cena (add/remove/update entity, undo/redo)
  - Sistema de componentes (registry, validacao, defaults)
  - Asset pipeline (import, index, metadata)
  - Serializacao (load/save scenes, scripts, project)
  - Preview runtime (spawn, IPC, lifecycle)
  - Projeto (create, open, config)
  - Console (log buffer, filter)
  - Selection state (selected IDs, multi-select)
  - Clipboard (copy/paste entities)
  - History (undo/redo command stack)

FRONTEND (TypeScript/React) — "View Layer"
  - Rendering de UI (paineis, botoes, inputs)
  - Viewport 3D (Three.js Canvas, render visual)
  - Viewport 2D (render visual)
  - Drag-and-drop visual (ghost, preview)
  - Animacoes de UI (framer-motion)
  - Temas/CSS
  - Keyboard shortcuts -> invocam comandos no backend
  - Polling do estado do backend e re-render
```

### Padrao de comunicacao (inspirado Unity)

```
[User Input] -> [Frontend Command] -> [Tauri invoke] -> [Backend executa]
                                                              |
                                                              v
                                                    [Backend emite evento]
                                                              |
                                                              v
                                                    [Frontend recebe e re-renderiza]
```

---

## 3. Arquitetura alvo

### Backend Rust — Estrutura de modulos

```
src-tauri/src/
  main.rs                    -- bootstrap Tauri, registra commands
  commands/
    mod.rs
    project.rs               -- open, create, save project
    scene.rs                 -- load, save, manipulate scene
    entity.rs                -- add, remove, update, duplicate, reparent
    component.rs             -- add, remove, update component
    asset.rs                 -- import, index, metadata, thumbnails
    preview.rs               -- start, stop, pause, poll runtime
    history.rs               -- undo, redo, clear
    selection.rs             -- select, deselect, multi-select
    clipboard.rs             -- copy, paste, cut
    console.rs               -- log, clear, filter
    editor.rs                -- preferences, keybinds, layout state
  models/
    mod.rs
    project.rs               -- ProjectConfig, StudioLayout, StudioHome
    scene.rs                 -- SceneDocument, SceneEnvironment, SceneRender, SceneAudio
    entity.rs                -- Entity, Transform3D, EntityId
    component.rs             -- Component, ComponentSchema, ComponentField
    asset.rs                 -- AssetEntry, AssetKind, AssetDatabase
    script.rs                -- ScriptDocument
    console.rs               -- ConsoleLine, ConsoleLevel
    preview.rs               -- PreviewSession, PreviewEvent, PreviewStatus
    selection.rs             -- Selection, SelectionSet
    history.rs               -- HistoryStack, Command (trait)
  services/
    mod.rs
    scene_manager.rs         -- orchestrates scene state, entity tree, component registry
    asset_pipeline.rs        -- scan, import, index, thumbnail generation
    component_registry.rs    -- schema registry, validation, defaults, creation
    history_manager.rs       -- command execution, undo stack, redo stack
    preview_runner.rs        -- child process lifecycle, JSONL IPC
    project_manager.rs       -- project lifecycle, templates, manifest
    file_service.rs          -- file I/O, watch, path resolution
  state/
    mod.rs
    app_state.rs             -- AppState struct centralizado (Mutex-wrapped)
  utils/
    mod.rs
    paths.rs                 -- normalize, resolve, slugify
    json.rs                  -- safe JSON helpers
```

### Frontend React — Estrutura de modulos

```
src/
  main.tsx                   -- ReactDOM.createRoot
  App.tsx                    -- shell layout (< 100 linhas)
  store/
    studioStore.ts           -- Zustand store (estado sincronizado com backend)
    selectors.ts             -- derived state selectors
  hooks/
    useBackendSync.ts        -- polling/eventos do backend
    useKeyboardShortcuts.ts  -- keyboard -> commands
    useViewportCamera.ts     -- camera state para viewport
    useDragDrop.ts           -- asset/entity drag logic
    useCommand.ts            -- invoca comandos no backend
  commands/
    index.ts                 -- command registry
    entityCommands.ts        -- addEntity, deleteEntity, duplicateEntity, etc
    sceneCommands.ts         -- saveScene, loadScene, etc
    selectionCommands.ts     -- select, deselect, selectAll
    historyCommands.ts       -- undo, redo
    previewCommands.ts       -- play, pause, stop
    projectCommands.ts       -- open, create, save
  components/
    layout/
      StudioShell.tsx        -- grid layout principal
      MenuBar.tsx
      ResizeHandle.tsx
    panels/
      navigator/
        NavigatorPanel.tsx
        EntityTree.tsx       -- arvore recursiva com parent-child
        EntityTreeNode.tsx
      inspector/
        InspectorPanel.tsx
        IdentitySection.tsx
        TransformSection.tsx
        ComponentsSection.tsx
        ComponentCard.tsx
        ComponentField.tsx
        SceneSettingsSection.tsx
        AddComponentMenu.tsx
      bottom/
        BottomDock.tsx
        AssetBrowser.tsx
        AssetTile.tsx
        ConsoleView.tsx
        ProblemsView.tsx
    viewport/
      SceneViewport.tsx      -- switch 2D/3D
      Viewport3D.tsx
      Viewport2D.tsx
      ViewportToolbar.tsx
      ViewportOverlay.tsx
      entities/
        Entity3DRenderer.tsx
        CameraGizmo.tsx
        LightGizmo.tsx
        AudioGizmo.tsx
        MeshRenderer.tsx
        SelectionOutline.tsx
      controls/
        TransformGizmo.tsx
        SnapController.tsx
        RadialMenu.tsx
        OrientationController.tsx
    editors/
      ScriptEditor.tsx
      ScriptEditorTabs.tsx
    dialogs/
      StartScreen.tsx
      SettingsDialog.tsx
      CommandPalette.tsx
      NewProjectDialog.tsx
    shared/
      StatusPill.tsx
      ToolButton.tsx
      PanelHeader.tsx
      InspectorGroup.tsx
      TextField.tsx
      VectorField.tsx
      DockTab.tsx
      SegmentedControl.tsx
  bridge/
    tauri.ts                 -- invoke wrappers tipados
    events.ts                -- Tauri event listeners
    types.ts                 -- tipos que espelham os do backend
  catalog/
    componentRegistry.ts     -- local mirror do registry do backend
    componentIcons.ts        -- mapeamento kind -> icone
    assetIcons.ts
  styles/
    variables.css            -- CSS custom properties
    reset.css                -- reset/normalize
    layout.css               -- grid do shell
    menu.css
    panels.css
    inspector.css
    viewport.css
    dialogs.css
    shared.css
    themes/
      codex.css
      xcode.css
      unity-dark.css
  types/
    index.ts                 -- re-exports
    scene.ts                 -- SceneDocument, SceneEntity, etc
    project.ts               -- ProjectConfig, ProjectAsset, etc
    editor.ts                -- StudioMode, ViewMode, Preferences, etc
    component.ts             -- ComponentSchema, ComponentField, etc
```

---

## 4. Roadmap de fases

```
FASE 0  Infraestrutura & tooling                        [1-2 dias]
FASE 1  Modularizacao do Backend Rust                    [3-4 dias]
FASE 2  Modularizacao do Frontend React                  [3-4 dias]
FASE 3  Motor de cena e entidades no backend             [2-3 dias]
FASE 4  Sistema de componentes real                      [2-3 dias]
FASE 5  Asset pipeline                                   [2-3 dias]
FASE 6  Undo/Redo e Selection                            [2 dias]
FASE 7  Viewport e rendering                             [3-4 dias]
FASE 8  Build e preview                                  [2-3 dias]
FASE 9  Editors especializados                           [3-4 dias]
FASE 10 Polish e release                                 [2-3 dias]
                                                    TOTAL: ~25-33 dias
```

---

## 5. Fase 0 — Infraestrutura

### 0.1 Dependencias

**Rust (Cargo.toml)**
```toml
[dependencies]
serde = { version = "1", features = ["derive"] }
serde_json = "1"
tauri = { version = "2", features = ["protocol-asset"] }
uuid = { version = "1", features = ["v4"] }
notify = "7"                    # file watching
toml = "0.8"                    # parse zenith.ztproj
```

**TypeScript (package.json) — adicionar**
```json
{
  "zustand": "^5.0",
  "immer": "^10.1",
  "@codemirror/lang-javascript": "^6.2",
  "codemirror": "^6.0",
  "@uiw/react-codemirror": "^4.23"
}
```

### 0.2 Config

- [ ] Adicionar `eslint` + `prettier` com regras de max linhas
- [ ] Adicionar `clippy` lints no Cargo.toml
- [ ] Criar `src-tauri/build.rs` para tauri-build
- [ ] Verificar `tsconfig.json` paths aliases (`@/` -> `src/`)

### 0.3 Testes

- [ ] Setup `cargo test` com modulos de teste
- [ ] Setup `vitest` para testes de frontend (unit)

---

## 6. Fase 1 — Modularizacao do Backend Rust

**Objetivo:** Quebrar `main.rs` (1664 linhas) em modulos coesos.

### 1.1 Models (extrair structs)

- [ ] Criar `src-tauri/src/models/mod.rs`
- [ ] Mover `StudioSnapshot`, `StudioHome`, `StudioLayout` -> `models/project.rs`
- [ ] Mover `SceneDocument`, `SceneEntity`, `Transform3d`, `SceneComponent` -> `models/scene.rs`
- [ ] Mover `ProjectAsset` -> `models/asset.rs`
- [ ] Mover `ScriptDocument` -> `models/script.rs`
- [ ] Mover `ConsoleLine` -> `models/console.rs`
- [ ] Mover `PreviewSession`, `PreviewEvent`, `PreviewCommandResult` -> `models/preview.rs`
- [ ] Mover `ProjectTemplate`, `ProjectTemplateAsset`, `DocumentationLink` -> `models/project.rs`
- [ ] Criar `models/selection.rs` — `SelectionState { selected_ids: HashSet<String>, primary_id: Option<String> }`
- [ ] Criar `models/history.rs` — `HistoryStack`, `EditCommand` trait

**Regra:** Todas as structs ganham `#[derive(Debug, Clone, Serialize, Deserialize)]` (Deserialize para receber do frontend).

### 1.2 Estado centralizado

- [ ] Criar `src-tauri/src/state/app_state.rs`:

```rust
pub struct AppState {
    pub layout: StudioLayout,
    pub project: Option<ProjectState>,
}

pub struct ProjectState {
    pub config: ProjectConfig,
    pub scene: SceneDocument,
    pub assets: Vec<ProjectAsset>,
    pub scripts: Vec<ScriptDocument>,
    pub selection: SelectionState,
    pub history: HistoryStack,
    pub console: Vec<ConsoleLine>,
    pub preview: PreviewState,
    pub dirty: bool,
}
```

- [ ] Usar `tauri::State<Mutex<AppState>>` em vez de `OnceLock<Mutex<Option<PreviewSession>>>`

### 1.3 Commands (extrair tauri commands)

- [ ] Criar `commands/project.rs`:
  - `load_studio_home` (mover de main.rs)
  - `load_studio_snapshot` (mover de main.rs)
  - `create_borealis_project` (mover de main.rs)
  - `save_project`

- [ ] Criar `commands/scene.rs`:
  - `save_scene` (mover lógica de write_scene de sceneSerialization.ts -> backend)
  - `load_scene`
  - `update_scene_settings`

- [ ] Criar `commands/entity.rs` **(NOVO — migra logica do frontend)**:
  - `add_entity(name, layer, tags, components, transform) -> Entity`
  - `remove_entity(entity_id)`
  - `update_entity(entity_id, patch)`
  - `duplicate_entity(entity_id) -> Entity`
  - `reparent_entity(entity_id, new_parent_id)`
  - `reorder_entity(entity_id, new_index)`
  - `update_transform(entity_id, transform)`

- [ ] Criar `commands/component.rs` **(NOVO)**:
  - `add_component(entity_id, kind) -> Component`
  - `remove_component(entity_id, component_index)`
  - `update_component(entity_id, component_index, field_key, value)`

- [ ] Criar `commands/selection.rs` **(NOVO)**:
  - `select_entity(entity_id)`
  - `deselect_all`
  - `toggle_selection(entity_id)`
  - `select_all`
  - `get_selection -> SelectionState`

- [ ] Criar `commands/history.rs` **(NOVO)**:
  - `undo -> SceneSnapshot`
  - `redo -> SceneSnapshot`
  - `get_history_state -> { can_undo: bool, can_redo: bool, description: String }`

- [ ] Criar `commands/clipboard.rs` **(NOVO)**:
  - `copy_entities(entity_ids)`
  - `paste_entities -> Vec<Entity>`
  - `cut_entities(entity_ids)`

- [ ] Criar `commands/preview.rs` (mover de main.rs):
  - `start_preview`
  - `pause_preview`
  - `stop_preview`
  - `poll_preview`

- [ ] Criar `commands/asset.rs` **(NOVO)**:
  - `scan_assets -> Vec<ProjectAsset>`
  - `import_asset(source_path) -> ProjectAsset`
  - `delete_asset(asset_id)`
  - `get_asset_metadata(asset_id) -> AssetMetadata`

- [ ] Criar `commands/console.rs` **(NOVO)**:
  - `get_console_lines(filter) -> Vec<ConsoleLine>`
  - `clear_console`
  - `append_console_line(level, source, message)`

- [ ] Criar `commands/editor.rs` **(NOVO)**:
  - `get_editor_state -> EditorSnapshot` (estado completo para sync)
  - `save_preferences`
  - `load_preferences`

### 1.4 Services (logica de negocio)

- [ ] Criar `services/scene_manager.rs`:
  - Logica de manipulacao do SceneDocument
  - Validacao de entidades
  - Serialization/deserialization
  - EntityTree (hierarquia)

- [ ] Criar `services/component_registry.rs`:
  - Mover logica de `borealisCatalog.ts` para Rust
  - Registry com schemas, defaults, validacao
  - `create_default_component(kind) -> Component`
  - `validate_component(component) -> Result`
  - `get_component_schema(kind) -> ComponentSchema`

- [ ] Criar `services/history_manager.rs`:
  - Command pattern: `trait EditCommand { execute, undo, description }`
  - Stack de comandos
  - Merge de comandos consecutivos (ex: multiplos moves vira um)

- [ ] Criar `services/asset_pipeline.rs`:
  - Scan recursivo da pasta do projeto
  - Classificacao por extensao (mover `asset_kind()` de main.rs)
  - Indice de assets com UUID estavel

- [ ] Criar `services/preview_runner.rs`:
  - Mover toda logica de spawn/IPC de main.rs
  - Lifecycle management

- [ ] Criar `services/project_manager.rs`:
  - Mover logica de create/load project
  - Templates
  - Manifest loading

- [ ] Criar `services/file_service.rs`:
  - I/O helpers
  - Path normalization (mover `normalize_path`, `resolve_project_path`, etc)

### 1.5 Resultado esperado

```
main.rs              ~40 linhas (bootstrap only)
commands/            ~8 arquivos, ~100-200 linhas cada
models/              ~8 arquivos, ~30-80 linhas cada
services/            ~7 arquivos, ~100-250 linhas cada
state/               ~1 arquivo, ~60 linhas
utils/               ~2 arquivos, ~50-80 linhas cada
```

**Teste de validacao:** `cargo build` compila, `cargo test` passa, todos os commands invocaveis do frontend.

---

## 7. Fase 2 — Modularizacao do Frontend React

**Objetivo:** Quebrar `App.tsx` (3644 linhas) e `styles.css` (2038 linhas) em modulos coesos.

### 2.1 State management

- [ ] Instalar Zustand
- [ ] Criar `store/studioStore.ts`:

```typescript
interface StudioStore {
  // Sincronizado do backend
  project: ProjectState | null;
  scene: SceneDocument | null;
  entities: SceneEntity[];
  assets: ProjectAsset[];
  scripts: ScriptDocument[];
  console: ConsoleLine[];
  selection: SelectionState;
  historyState: { canUndo: boolean; canRedo: boolean; description: string };
  previewStatus: PreviewStatus;

  // Estado local de UI (nao vai para o backend)
  bridgeStatus: BridgeStatus;
  viewMode: ViewMode;
  mode: StudioMode;
  bottomTab: BottomTab;
  layout: LayoutDimensions;
  preferences: StudioPreferences;
  homeVisible: boolean;
  scriptEditorOpen: boolean;
  settingsOpen: boolean;

  // Actions que invocam o backend
  openProject: (path: string) => Promise<void>;
  createProject: (request: NewProjectRequest) => Promise<void>;
  selectEntity: (entityId: string) => Promise<void>;
  deselectAll: () => Promise<void>;
  addEntity: (name: string, layer: string) => Promise<void>;
  deleteEntity: (entityId: string) => Promise<void>;
  updateTransform: (entityId: string, transform: Transform3D) => Promise<void>;
  addComponent: (entityId: string, kind: string) => Promise<void>;
  removeComponent: (entityId: string, index: number) => Promise<void>;
  saveScene: () => Promise<void>;
  undo: () => Promise<void>;
  redo: () => Promise<void>;
  startPreview: () => Promise<void>;
  stopPreview: () => Promise<void>;
  syncFromBackend: () => Promise<void>;

  // Actions locais de UI
  setViewMode: (mode: ViewMode) => void;
  setMode: (mode: StudioMode) => void;
  setBottomTab: (tab: BottomTab) => void;
  setLayout: (layout: Partial<LayoutDimensions>) => void;
}
```

- [ ] Criar `store/selectors.ts`:

```typescript
export const selectedEntity = (state: StudioStore) =>
  state.entities.find(e => state.selection.primaryId === e.id) ?? null;

export const selectedEntities = (state: StudioStore) =>
  state.entities.filter(e => state.selection.selectedIds.has(e.id));

export const entityTree = (state: StudioStore) =>
  buildEntityTree(state.entities);

export const activeScript = (state: StudioStore) => ...;
```

### 2.2 Tipos

- [ ] Mover tipos de `types.ts` para `types/` com separacao:
  - `types/scene.ts` — SceneDocument, SceneEntity, SceneComponent, Transform3D, SceneEnvironment, SceneRender, SceneAudio
  - `types/project.ts` — ProjectAsset, AssetKind, ScriptDocument, ProjectTemplate
  - `types/editor.ts` — StudioMode, ViewMode, PreviewStatus, BridgeStatus, Preferences, DragState
  - `types/component.ts` — ComponentSchema, ComponentFieldSchema
  - `types/index.ts` — re-exports

### 2.3 Bridge (Tauri invoke layer)

- [ ] Reescrever `backend.ts` -> `bridge/tauri.ts` com tipagem forte:

```typescript
// Cada funcao mapeia 1:1 para um tauri::command
export async function invokeAddEntity(params: AddEntityParams): Promise<SceneEntity> {
  return invoke<SceneEntity>("add_entity", params);
}

export async function invokeUndo(): Promise<EditorSnapshot> {
  return invoke<EditorSnapshot>("undo");
}
// ... etc para cada command do backend
```

- [ ] Criar `bridge/events.ts` para Tauri event listeners

### 2.4 Componentes — Extracao de App.tsx

**Ordem de extracao (de dentro para fora, bottom-up):**

#### Shared (sem dependencias)
- [ ] `shared/StatusPill.tsx` (linhas 3347-3349)
- [ ] `shared/ToolButton.tsx` (linhas 3321-3345)
- [ ] `shared/PanelHeader.tsx` (linhas 3186-3207)
- [ ] `shared/InspectorGroup.tsx` (linhas 3218-3230)
- [ ] `shared/TextField.tsx` (linhas 3232-3247)
- [ ] `shared/VectorField.tsx` (linhas 3249-3280)
- [ ] `shared/SegmentedControl.tsx` (linhas 3282-3300)
- [ ] `shared/DockTab.tsx` (linhas 3302-3319)

#### Inspector
- [ ] `panels/inspector/ComponentField.tsx` (linhas 2862-2953)
- [ ] `panels/inspector/ComponentCard.tsx` (linhas 2802-2860)
- [ ] `panels/inspector/SceneSettingsSection.tsx` (linhas 2741-2800)
- [ ] `panels/inspector/AddComponentMenu.tsx` (extrair de linhas 2542-2562)
- [ ] `panels/inspector/IdentitySection.tsx` (extrair de InspectorPanel)
- [ ] `panels/inspector/TransformSection.tsx` (extrair de InspectorPanel)
- [ ] `panels/inspector/ComponentsSection.tsx` (extrair de InspectorPanel)
- [ ] `panels/inspector/InspectorPanel.tsx` (linhas 2450-2583, refatorado)

#### Navigator
- [ ] `panels/navigator/EntityTreeNode.tsx` (novo — node recursivo)
- [ ] `panels/navigator/EntityTree.tsx` (novo — arvore completa)
- [ ] `panels/navigator/NavigatorPanel.tsx` (linhas 1317-1355, refatorado)

#### Bottom Dock
- [ ] `panels/bottom/AssetTile.tsx` (linhas 3099-3135)
- [ ] `panels/bottom/AssetDragPreview.tsx` (linhas 3137-3150)
- [ ] `panels/bottom/AssetBrowser.tsx` (extrair de BottomDock)
- [ ] `panels/bottom/ConsoleView.tsx` (linhas 3152-3163)
- [ ] `panels/bottom/ProblemsView.tsx` (linhas 3165-3184)
- [ ] `panels/bottom/BottomDock.tsx` (linhas 3000-3097, refatorado)

#### Viewport
- [ ] `viewport/entities/CameraGizmo.tsx` (extrair de ViewportEntityGizmo)
- [ ] `viewport/entities/LightGizmo.tsx` (extrair de ViewportEntityGizmo)
- [ ] `viewport/entities/AudioGizmo.tsx` (extrair de ViewportEntityGizmo)
- [ ] `viewport/entities/MeshRenderer.tsx` (extrair de ViewportEntityGizmo)
- [ ] `viewport/entities/SelectionOutline.tsx` (linhas 2706-2739)
- [ ] `viewport/entities/Entity3DRenderer.tsx` (linhas 2094-2236, refatorado)
- [ ] `viewport/entities/SceneLights.tsx` (linhas 2038-2092)
- [ ] `viewport/controls/OrientationController.tsx` (linhas 1968-2036)
- [ ] `viewport/controls/RadialMenu.tsx` (extrair de Viewport3D)
- [ ] `viewport/controls/SnapPanel.tsx` (extrair de Viewport3D)
- [ ] `viewport/controls/ViewControls.tsx` (extrair de Viewport3D)
- [ ] `viewport/ViewportOverlay.tsx` (extrair overlays de Viewport3D)
- [ ] `viewport/Viewport3D.tsx` (linhas 1632-1966, refatorado)
- [ ] `viewport/Viewport2D.tsx` (linhas 2254-2309, refatorado)
- [ ] `viewport/ViewportToolbar.tsx` (linhas 1357-1447 StageToolbar, renomear)
- [ ] `viewport/SceneViewport.tsx` (linhas 1449-1614, refatorado)

#### Editors
- [ ] `editors/ScriptEditorTabs.tsx` (extrair tabs)
- [ ] `editors/ScriptEditor.tsx` (linhas 2321-2448, refatorado com CodeMirror)

#### Dialogs
- [ ] `dialogs/StartScreen.tsx` (linhas 873-1067)
- [ ] `dialogs/SettingsDialog.tsx` (linhas 1079-1185)
- [ ] `dialogs/CommandPalette.tsx` (linhas 1236-1315)

#### Layout
- [ ] `layout/MenuBar.tsx` (linhas 832-871)
- [ ] `layout/ResizeHandle.tsx` (extrair logica de resize)
- [ ] `layout/StudioShell.tsx` (contem o grid layout, workspace)

#### App.tsx final
- [ ] Refatorar `App.tsx` para < 100 linhas:
  - Inicializacao do store
  - StudioShell como unico filho
  - useBackendSync hook

### 2.5 CSS Modularizacao

- [ ] Criar `styles/variables.css` (CSS custom properties — linhas 1-29 do styles.css atual)
- [ ] Criar `styles/reset.css` (linhas 31-50)
- [ ] Criar `styles/layout.css` (studio-shell, studio-workspace, grid)
- [ ] Criar `styles/menu.css` (menu-bar, menu-items)
- [ ] Criar `styles/panels.css` (side-panel, navigator, bottom-dock)
- [ ] Criar `styles/inspector.css` (inspector-panel, component-card, fields)
- [ ] Criar `styles/viewport.css` (scene-viewport, viewport-*, gizmo)
- [ ] Criar `styles/dialogs.css` (start-screen, settings, script-window)
- [ ] Criar `styles/shared.css` (status-pill, tool-button, etc)
- [ ] Criar `styles/themes/codex.css`, `xcode.css`, `unity-dark.css`
- [ ] Import chain em `main.tsx` ou `styles/index.css`

### 2.6 Hooks

- [ ] `hooks/useBackendSync.ts` — polling periodic do backend state
- [ ] `hooks/useKeyboardShortcuts.ts` — mover logica de keyboard de App.tsx
- [ ] `hooks/useViewportCamera.ts` — mover camera state de SceneViewport
- [ ] `hooks/useDragDrop.ts` — mover asset drag logic de App.tsx
- [ ] `hooks/useResizePanel.ts` — mover resize logic de App.tsx
- [ ] `hooks/useCommand.ts` — wrapper para invocar commands

### 2.7 Helpers

- [ ] Mover funcoes helper de App.tsx para utils:
  - `utils/entityHelpers.ts` — entityClass, entityGlyph, entityIcon, entityScript, entityBounds
  - `utils/projections.ts` — project2d, project3d
  - `utils/snap.ts` — snapTransform, snapValue, gridMetersToSceneUnits
  - `utils/formatting.ts` — formatComponentValue, componentInputValue
  - `utils/sceneHelpers.ts` — sceneColorValue, sceneNumberValue, sceneBooleanValue, sceneSettingSection, sceneSettingValues
  - `utils/clamp.ts`

**Teste de validacao:** `npm run dev` inicia sem erros, todas as features existentes funcionam identicamente.

---

## 8. Fase 3 — Motor de cena e entidades no backend

**Objetivo:** O backend Rust e a autoridade sobre a cena. O frontend nunca modifica entidades diretamente.

### 3.1 EntityTree no Rust

- [ ] Implementar `SceneManager` com:
  - `entities: HashMap<EntityId, Entity>` (lookup por ID)
  - `root_order: Vec<EntityId>` (ordem de raiz)
  - `children: HashMap<EntityId, Vec<EntityId>>` (hierarquia)

- [ ] Operacoes:
  - `add_entity(parent: Option<EntityId>, config) -> EntityId` — gera UUID, insere na arvore
  - `remove_entity(id) -> Vec<Entity>` — remove recursivamente filhos
  - `reparent(id, new_parent, index)` — muda hierarquia
  - `reorder(id, new_index)` — reordena entre siblings
  - `get_entity(id) -> &Entity`
  - `get_children(id) -> &[EntityId]`
  - `flatten_tree() -> Vec<FlatTreeNode>` — para o frontend renderizar

### 3.2 Entity ID

- [ ] Usar UUID v4 para entity IDs (nao mais `entity-01` sequencial)
- [ ] IDs estaveis que sobrevivem save/load

### 3.3 Transform como first-class

- [ ] Transform3D com operacoes no Rust:
  - Snap to grid
  - Snap to object
  - Clamp scale
  - Local vs world space (futuro)

### 3.4 Frontend sync

- [ ] Frontend recebe `FlatEntityTree` do backend (array flat com depth info)
- [ ] Frontend nunca faz `setSnapshot(current => ...)` para entidades
- [ ] Toda mutacao: `invoke("update_entity", { entityId, patch })` -> backend processa -> frontend recebe novo estado

---

## 9. Fase 4 — Sistema de componentes real

**Objetivo:** Registry de componentes no backend com schema, validacao, defaults e criacao tipada. Inspirado em Unity `AddComponent`.

### 4.1 ComponentRegistry no Rust

- [ ] Mover TODA logica de `borealisCatalog.ts` para `services/component_registry.rs`
- [ ] Registry carregado de:
  1. Built-in schemas (hardcoded no Rust)
  2. `borealis.editor.json` (override/extend)

- [ ] Cada schema define:

```rust
pub struct ComponentSchema {
    pub kind: String,
    pub label: String,
    pub description: String,
    pub category: ComponentCategory,  // Transform, Rendering, Physics, Audio, Script, Custom
    pub fields: Vec<ComponentFieldSchema>,
    pub icon: String,                 // nome do icone para o frontend
    pub singleton: bool,              // max 1 por entity (ex: Transform, Rigidbody)
    pub requires: Vec<String>,        // dependencias (ex: Collider requer Rigidbody)
}

pub enum ComponentCategory {
    Rendering,
    Physics,
    Audio,
    Camera,
    Light,
    Script,
    AI,
    UI,
    Custom,
}
```

### 4.2 Validacao

- [ ] Ao adicionar componente: verificar `singleton` (nao duplicar), verificar `requires` (adicionar dependencias automaticamente)
- [ ] Ao remover: verificar se outro componente depende dele (warn)
- [ ] Validacao de campo: tipo, min/max, opcoes validas

### 4.3 Frontend

- [ ] `borealisCatalog.ts` vira thin client que busca schemas do backend
- [ ] `AddComponentMenu` mostra categorias (como Unity: Rendering, Physics, Audio, etc)
- [ ] Inspector renderiza fields com base no schema recebido

---

## 10. Fase 5 — Asset pipeline

**Objetivo:** Assets sao gerenciados pelo backend com indice, metadata e import flow.

### 5.1 AssetDatabase no Rust

- [ ] `AssetEntry`:

```rust
pub struct AssetEntry {
    pub id: String,          // UUID estavel
    pub name: String,
    pub path: String,        // relativo ao projeto
    pub kind: AssetKind,
    pub size_bytes: u64,
    pub modified: u64,       // timestamp
    pub metadata: Value,     // tipo-especifico
}
```

- [ ] Scan recursivo de `assets/`, `scenes/`, `src/` ao abrir projeto
- [ ] Indice persistido em `.borealis/asset_index.json` (cache)

### 5.2 Import flow

- [ ] Command `import_asset(source_path, target_dir)`:
  1. Copia arquivo para `assets/`
  2. Gera UUID
  3. Classifica tipo
  4. Extrai metadata (dimensoes para texturas, vertex count para modelos, duracao para audio)
  5. Retorna `AssetEntry`

### 5.3 File watching (futuro)

- [ ] Usar crate `notify` para watch de `assets/` e `src/`
- [ ] Hot-reload de assets modificados externamente
- [ ] Emitir evento Tauri para frontend atualizar

### 5.4 Frontend

- [ ] AssetBrowser busca lista do backend
- [ ] Double-click em modelo: `invoke("create_entity_from_asset", { assetId })`
- [ ] Drag-drop: frontend envia asset ID, backend cria entidade

---

## 11. Fase 6 — Undo/Redo e Selection

**Objetivo:** Command pattern no backend, toda operacao e reversivel.

### 6.1 Command trait

```rust
pub trait EditCommand: Send + Sync {
    fn execute(&self, state: &mut ProjectState) -> Result<(), String>;
    fn undo(&self, state: &mut ProjectState) -> Result<(), String>;
    fn description(&self) -> String;
    fn merge_with(&self, other: &dyn EditCommand) -> Option<Box<dyn EditCommand>> {
        None // default: nao faz merge
    }
}
```

### 6.2 Comandos implementados

- [ ] `AddEntityCommand { entity: Entity }`
- [ ] `RemoveEntityCommand { entity: Entity, index: usize }`
- [ ] `UpdateEntityCommand { entity_id, old_patch, new_patch }`
- [ ] `UpdateTransformCommand { entity_id, old_transform, new_transform }`
  - merge: transforms consecutivos do mesmo entity viram um
- [ ] `AddComponentCommand { entity_id, component }`
- [ ] `RemoveComponentCommand { entity_id, component, index }`
- [ ] `UpdateComponentCommand { entity_id, index, old_component, new_component }`
- [ ] `ReparentEntityCommand { entity_id, old_parent, new_parent }`
- [ ] `UpdateSceneSettingsCommand { section, old_values, new_values }`
- [ ] `BatchCommand { commands: Vec<Box<dyn EditCommand>> }` — grupo atomico

### 6.3 History stack

- [ ] `undo_stack: Vec<Box<dyn EditCommand>>` (max 100)
- [ ] `redo_stack: Vec<Box<dyn EditCommand>>`
- [ ] Toda mutacao: push no undo_stack, limpa redo_stack
- [ ] Merge automatico: se ultimo command e mesmo tipo + mesmo entity + < 500ms -> merge

### 6.4 Selection

- [ ] `SelectionState`:

```rust
pub struct SelectionState {
    pub selected_ids: HashSet<String>,
    pub primary_id: Option<String>,       // ultimo selecionado
    pub hover_id: Option<String>,          // hover visual
}
```

- [ ] Commands:
  - Click: select single
  - Ctrl+Click: toggle na selecao
  - Shift+Click: range select (na arvore)
  - Ctrl+A: select all
  - Delete: remove selected
  - Ctrl+D: duplicate selected
  - Ctrl+C/V/X: copy/paste/cut

### 6.5 Frontend

- [ ] Ctrl+Z -> `invoke("undo")` -> recebe novo state -> re-render
- [ ] Ctrl+Y / Ctrl+Shift+Z -> `invoke("redo")`
- [ ] StatusBar mostra "Undo: [description]" e "Redo: [description]"
- [ ] Multi-select visual no Navigator e Viewport

---

## 12. Fase 7 — Viewport e rendering

**Objetivo:** Viewport 3D renderiza assets reais, nao apenas placeholders.

### 7.1 Model loading

- [ ] Usar `useGLTF` / `useLoader(OBJLoader)` do drei para carregar modelos reais
- [ ] Fallback: cubo colorido se modelo nao existe
- [ ] Cache de modelos carregados

### 7.2 Texture loading

- [ ] Sprites/texturas como `<Sprite>` ou plane com texture
- [ ] Preview de material no entity

### 7.3 Entity rendering refactor

- [ ] `Entity3DRenderer` decide o que renderizar com base nos componentes:
  - `model3d` -> carrega .glb/.obj real
  - `cube3d` -> BoxGeometry
  - `sphere3d` -> SphereGeometry
  - `plane3d` -> PlaneGeometry
  - `camera3d` -> CameraGizmo (wireframe frustum)
  - `directional_light` -> LightGizmo (direcional)
  - `point_light` -> LightGizmo (esfera range)
  - `spot_light` -> LightGizmo (cone)
  - `audio3d` -> AudioGizmo (esfera range + icone)
  - `sprite` -> Sprite com textura
  - `tilemap` -> TilemapPreview

### 7.4 Viewport 2D refactor

- [ ] Usar Canvas 2D (HTML Canvas ou PixiJS) em vez de CSS absolute
- [ ] Grid real, zoom real, pan real
- [ ] Entity rendering com sprites/shapes

### 7.5 Gizmos

- [ ] Selection box (marquee select com mouse drag)
- [ ] Snap guides visuais (linhas quando snap acontece)
- [ ] Bounds visualization (wireframe box por entity)

---

## 13. Fase 8 — Build e preview

### 8.1 Preview refactor

- [ ] Mover TODA logica de preview para `services/preview_runner.rs`
- [ ] Status machine:

```
Idle -> Starting -> Loading -> Ready -> Playing <-> Paused -> Stopped
                                                           -> Error
```

- [ ] IPC melhorado: JSONL bidirecional com request/response IDs
- [ ] Hot-reload: enviar `reload_scene` sem restart do processo

### 8.2 Build pipeline (futuro)

- [ ] Command `build_project(profile)`:
  1. Compila `.zt` -> C via compilador Zenith
  2. Compila C -> executavel nativo
  3. Empacota assets
  4. Output em `build/`
- [ ] Profiles: debug (rapido, sem otimizacao), release (otimizado)

---

## 14. Fase 9 — Editors especializados

### 9.1 Script Editor

- [ ] Integrar CodeMirror 6 (substitui textarea)
- [ ] Syntax highlighting para `.zt` (custom grammar)
- [ ] Autocomplete basico (keywords, funcoes built-in)
- [ ] Error markers (linhas com erro)
- [ ] Line numbers, bracket matching, auto-indent
- [ ] Multi-tab com dirty indicator

### 9.2 Command Palette

- [ ] Implementar busca real:
  - Entidades por nome
  - Assets por nome
  - Scripts por nome
  - Commands registrados (Save, Undo, Redo, Add Entity, etc)
- [ ] Keyboard: Ctrl+Shift+P (como VS Code/Unity)
- [ ] Fuzzy search

### 9.3 Console melhorado

- [ ] Filtro por level (info, warn, error)
- [ ] Filtro por source (studio, preview, script)
- [ ] Clear button
- [ ] Auto-scroll com lock
- [ ] Timestamp
- [ ] Contadores de warn/error no tab

### 9.4 Futuro (nao blocking)

- [ ] Animation timeline editor
- [ ] Tilemap editor
- [ ] Particle editor
- [ ] Material editor

---

## 15. Fase 10 — Polish e release

- [ ] Performance audit (React DevTools, Rust profiling)
- [ ] Accessibility (ARIA, focus management, screen reader)
- [ ] i18n prep (externalizar strings)
- [ ] Error boundaries em todos os paineis
- [ ] Loading states e skeletons
- [ ] Empty states com acoes uteis
- [ ] Tooltips em todos os botoes/controles
- [ ] Documentacao de contribuicao
- [ ] Tests de integracao (Tauri + Frontend)
- [ ] CI/CD para build automatico

---

## 16. Checklist master

### Fase 0 — Infraestrutura
- [ ] 0.1 Adicionar dependencias Rust (uuid, notify, toml)
- [ ] 0.2 Adicionar dependencias TS (zustand, immer, codemirror)
- [ ] 0.3 Configurar eslint + prettier
- [ ] 0.4 Configurar clippy lints
- [ ] 0.5 Path aliases no tsconfig
- [ ] 0.6 Setup cargo test
- [ ] 0.7 Setup vitest

### Fase 1 — Backend Rust
- [ ] 1.1 Criar estrutura de pastas (models/, commands/, services/, state/, utils/)
- [ ] 1.2 Extrair models de main.rs (8 arquivos)
- [ ] 1.3 Criar AppState centralizado
- [ ] 1.4 Extrair commands de main.rs (project, scene, preview)
- [ ] 1.5 Criar novos commands (entity, component, selection, history, clipboard, asset, console, editor)
- [ ] 1.6 Extrair services (scene_manager, component_registry, history_manager, asset_pipeline, preview_runner, project_manager, file_service)
- [ ] 1.7 Extrair utils (paths, json)
- [ ] 1.8 Refatorar main.rs para < 50 linhas
- [ ] 1.9 cargo build compila
- [ ] 1.10 cargo test passa
- [ ] 1.11 Todos os commands invocaveis do frontend

### Fase 2 — Frontend React
- [ ] 2.1 Criar store Zustand
- [ ] 2.2 Criar selectors
- [ ] 2.3 Separar tipos em types/
- [ ] 2.4 Reescrever bridge/tauri.ts
- [ ] 2.5 Extrair shared components (8 componentes)
- [ ] 2.6 Extrair Inspector (8 componentes)
- [ ] 2.7 Extrair Navigator (3 componentes)
- [ ] 2.8 Extrair BottomDock (6 componentes)
- [ ] 2.9 Extrair Viewport (15+ componentes)
- [ ] 2.10 Extrair Editors (2 componentes)
- [ ] 2.11 Extrair Dialogs (4 componentes)
- [ ] 2.12 Extrair Layout (3 componentes)
- [ ] 2.13 Extrair hooks (6 hooks)
- [ ] 2.14 Extrair utils (6 modulos)
- [ ] 2.15 Modularizar CSS (10+ arquivos)
- [ ] 2.16 App.tsx < 100 linhas
- [ ] 2.17 npm run dev sem erros
- [ ] 2.18 Todas as features existentes funcionam

### Fase 3 — Motor de cena
- [ ] 3.1 SceneManager no Rust
- [ ] 3.2 EntityTree com hierarquia
- [ ] 3.3 UUID v4 para entity IDs
- [ ] 3.4 Transform operations no backend
- [ ] 3.5 Frontend sync via FlatEntityTree
- [ ] 3.6 Zero mutacao de entidades no frontend

### Fase 4 — Componentes
- [ ] 4.1 ComponentRegistry no Rust
- [ ] 4.2 Schemas com categories, singleton, requires
- [ ] 4.3 Validacao no backend
- [ ] 4.4 Frontend busca schemas do backend
- [ ] 4.5 AddComponentMenu com categorias

### Fase 5 — Assets
- [ ] 5.1 AssetDatabase no Rust
- [ ] 5.2 Scan ao abrir projeto
- [ ] 5.3 Import flow
- [ ] 5.4 Asset metadata extraction
- [ ] 5.5 Cache index persistido

### Fase 6 — Undo/Redo
- [ ] 6.1 EditCommand trait
- [ ] 6.2 10+ commands implementados
- [ ] 6.3 History stack (max 100)
- [ ] 6.4 Merge de commands consecutivos
- [ ] 6.5 Selection state no backend
- [ ] 6.6 Multi-select (Ctrl+Click, Shift+Click)
- [ ] 6.7 Delete/Duplicate selected
- [ ] 6.8 Copy/Paste/Cut
- [ ] 6.9 Ctrl+Z / Ctrl+Y funcionando end-to-end

### Fase 7 — Viewport
- [ ] 7.1 Model loading real (.glb, .obj)
- [ ] 7.2 Texture loading real
- [ ] 7.3 Entity rendering por tipo de componente
- [ ] 7.4 Viewport 2D com Canvas real
- [ ] 7.5 Marquee select
- [ ] 7.6 Snap guides visuais

### Fase 8 — Build/Preview
- [ ] 8.1 Preview como state machine
- [ ] 8.2 Hot-reload de cena
- [ ] 8.3 IPC bidirecional
- [ ] 8.4 Build pipeline basico

### Fase 9 — Editors
- [ ] 9.1 CodeMirror no script editor
- [ ] 9.2 Syntax highlighting para .zt
- [ ] 9.3 Command palette funcional com fuzzy search
- [ ] 9.4 Console com filtros e contadores

### Fase 10 — Polish
- [ ] 10.1 Performance audit
- [ ] 10.2 Accessibility audit
- [ ] 10.3 Error boundaries
- [ ] 10.4 Loading/empty states
- [ ] 10.5 Tooltips
- [ ] 10.6 Testes de integracao

---

## 17. Referencia Unity

### Mapeamento de conceitos Unity -> Borealis Studio

| Unity | Borealis Studio | Responsavel |
|---|---|---|
| `GameObject` | `SceneEntity` | Backend |
| `Component` (MonoBehaviour) | `SceneComponent` | Backend |
| `Transform` | `Transform3D` (integrado em Entity) | Backend |
| `Scene` (.unity) | `SceneDocument` (.scene.json) | Backend |
| `Prefab` | (futuro) `EntityTemplate` | Backend |
| `ScriptableObject` | (futuro) `DataAsset` | Backend |
| `AssetDatabase` | `AssetPipeline` | Backend |
| `Undo.RecordObject` | `EditCommand` trait | Backend |
| `Selection.activeGameObject` | `SelectionState` | Backend |
| `EditorGUI` / `PropertyDrawer` | `ComponentField` + schema | Frontend |
| `Hierarchy` window | `NavigatorPanel` + `EntityTree` | Frontend |
| `Inspector` window | `InspectorPanel` | Frontend |
| `Project` window | `AssetBrowser` | Frontend |
| `Console` window | `ConsoleView` | Frontend |
| `Scene` view | `Viewport3D` / `Viewport2D` | Frontend |
| `Game` view | Preview (sidecar process) | Backend + Frontend |
| `Play/Pause/Stop` | Preview controls | Backend |
| `AddComponent` menu | `AddComponentMenu` com categorias | Frontend + Backend schema |
| `Editor.SaveScene` | `invoke("save_scene")` | Backend |

### Padroes Unity que devem ser seguidos

1. **Tudo e componente** — sem entidade "especial", comportamento vem de componentes
2. **Inspector e generico** — qualquer componente renderiza seus campos automaticamente via schema
3. **Undo e automatico** — toda operacao do editor grava undo
4. **Hierarchy e drag-reorder** — reordenar por drag, reparent por drag
5. **Multi-select e pervasivo** — Inspector mostra campos comuns de multiplas entidades
6. **Play mode e isolado** — mudancas no play mode nao persistem
7. **Asset import e automatico** — dropar arquivo na pasta -> importado automaticamente
8. **Console filtra por tipo** — info/warn/error com contadores

---

*Este documento deve ser atualizado conforme cada fase e concluida.*
*Cada item da checklist marcado `[ ]` se torna `[x]` ao ser implementado.*
