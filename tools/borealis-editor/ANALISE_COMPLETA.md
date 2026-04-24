# Borealis Editor - Análise Completa do Projeto

**Data**: 2026-04-23  
**Versão**: 1.0  
**Status**: Fase 1 (Bridge Funcional) - Pendente

---

## Visão Geral do Ecossistema

```
┌─────────────────────────────────────────────────────────────────┐
│                     ZENITH LANGUAGE                              │
│                    (v0.3.0-alpha.1)                              │
│  Compiler: lexer → parser → AST → semantic → HIR → ZIR → C      │
│  Runtime: C runtime com ARC, isolate-single-thread              │
└─────────────────────────────────────────────────────────────────┘
                              │
                              │ usa
                              ▼
┌─────────────────────────────────────────────────────────────────┐
│                     BOREALIS PACKAGE                             │
│                    (game lib 2D/3D)                              │
│  - borealis.game (camada fácil)                                 │
│  - borealis.engine (camada técnica)                              │
│  - 20+ módulos 2D + 18 módulos 3D                               │
│  - Backend: Raylib (dinâmico) + fallback stub                   │
└─────────────────────────────────────────────────────────────────┘
                              │
                              │ será editado por
                              ▼
┌─────────────────────────────────────────────────────────────────┐
│                    BOREALIS EDITOR                               │
│                   (Rust + egui/eframe)                           │
│  - Editor desktop separado do runtime                           │
│  - IPC: JSONL sobre stdio                                       │
│  - Preview roda Borealis runtime real                           │
│  - Mesma estrutura de dados (editor ↔ runtime)                  │
└─────────────────────────────────────────────────────────────────┘
```

---

## Filosofia de Desenvolvimento

### **Princípio #1: Editor-Ready desde o Início**
*(Decision 005)*

O runtime Borealis foi construído **já pensando no editor futuro**:

| Requisito | Implementação |
|-----------|---------------|
| Entidades com IDs estáveis | `entities.Entity` com stable_id |
| Cenas como documentos | JSON serializável |
| Componentes serializáveis | Payloads em JSON |
| Metadata de editor separada | Não vaza para saves de jogo |
| Undo/redo como comandos | Command-based editing |
| Hot reload | Asset/scene/config reload |

**Impacto no editor**: O editor não precisa inventar formatos novos. Ele consome os **mesmos documentos** que o runtime.

---

### **Princípio #2: Dual-Layer Architecture**
*(Decision 006 + Stack)*

```
borealis.game     ← Camada fácil (onboarding)
    │
    └─> chama APIs simples: game.draw_rect(), game.input.key_pressed()
    
borealis.engine   ← Camada técnica (controle explícito)
    │
    └─> chama APIs avançadas: engine.ecs.system(), engine.backend.*()
```

**Impacto no editor**: 
- O editor pode usar `borealis.game` para preview simples
- Pode usar `borealis.engine` para ferramentas avançadas (inspector, ECS view)

---

### **Princípio #3: Process Model Separado**
*(Decision 011, 013)*

```
┌─────────────────────┐         ┌─────────────────────┐
│   EDITOR PROCESS    │         │   PREVIEW PROCESS   │
│   (Rust + egui)     │         │  (Borealis Runtime) │
│                     │         │   (Zenith + C)      │
│  - UI/panels        │         │  - Render loop      │
│  - scene tree       │ ◄─────► │  - Physics step     │
│  - inspector        │   IPC   │  - Game logic       │
│  - asset browser    │  JSONL  │  - Input handling   │
│  - undo/redo        │  stdio  │  - Audio            │
│  - Flow editor      │         │                     │
└─────────────────────┘         └─────────────────────┘
```

**Vantagens**:
1. Preview crash não derruba o editor
2. Editor state fica limpo enquanto jogo roda
3. Runtime fica próximo do comportamento real de produção
4. Fácil restart/reload/debug

---

## Estado Atual por Camada

### **1. Linguagem Zenith** ✅ Estável
| Componente | Status |
|------------|--------|
| Compiler | ✅ Pipeline completo (lexer → C → native) |
| Runtime | ✅ ARC, single-isolate, fallback seguro |
| Tooling | ✅ `zt check/build/run/fmt/doc/test` |
| Public var | ✅ Implementado (2026-04-22) |
| Stdlib | ✅ `std.io`, `std.json`, `std.random`, etc. |

**Bloqueios**: Backend C para structs tipadas e optionals (afeta Borealis)

---

### **2. Borealis Package** 🟡 85% Pronto

#### **2D Modules** (20 módulos)
| Categoria | Status |
|-----------|--------|
| Core (`game`, `engine`, `contracts`) | ✅ Concluído |
| Runtime (`entities`, `movement`, `controllers`) | ✅ Concluído |
| Render (`render2d`, `camera`, `input`) | ✅ Concluído |
| Audio/Animation | ✅ Concluído |
| World/Procedural | ✅ Concluído |
| UI/Scene/Events | ✅ Concluído |
| Save/Storage/Services | ✅ Concluído |
| Backend Raylib | 🟡 Parcial (fallback stub funciona) |

#### **3D Modules** (18 módulos)
| Categoria | Status |
|-----------|--------|
| Core/Render/Camera/Physics | 📐 Documentado + scaffold |
| World/Terrain/Chunks | 📐 Documentado + scaffold |
| Assets/Animation/Audio | 📐 Documentado |
| AI/Controllers/UI3D | 📐 Documentado |
| PostFX/Debug3D | 📐 Documentado |

**Bloqueios**: Implementação real depende de backend C maduro

---

### **3. Borealis Editor** 🟡 40% Pronto

#### **Implementado** ✅
```rust
tools/borealis-editor/
├── Cargo.toml (eframe 0.33, serde, serde_json, tracing)
├── src/
│   ├── main.rs (entry point, logging setup)
│   ├── app.rs (BorealisEditorApp - UI com painéis)
│   ├── ipc.rs (PreviewBridge - transporte JSONL-stdio)
│   └── messages.rs (PreviewEnvelope, PreviewPayload)
```

**Funcionalidades**:
- ✅ App desktop com `eframe`
- ✅ UI: top bar, left panel, central panel
- ✅ Protocolo IPC v1 (envelope versionado)
- ✅ Bridge de preview com estados (Idle/Starting/Ready/Playing/Stopped)
- ✅ Mensagens: Hello, OpenScene, EnterPlayMode, SelectEntity, Ping/Pong, Status, Diagnostic
- ✅ Mock do IPC (simula comunicação sem preview real)
- ✅ Transcript de mensagens (debug)

**UI Atual**:
```
┌──────────────────────────────────────────────────────────┐
│ Borealis Editor | Stack: Rust+egui | Transport: Jsonl... │
├──────────────┬───────────────────────────────────────────┤
│              │                                           │
│ Preview      │  Viewport Placeholder                     │
│ Bridge       │  (Preview sidecar will live here later)   │
│              │                                           │
│ [Start]      ├───────────────────────────────────────────┤
│ [Open Scene] │ Transcript                                │
│ [Play Mode]  │ editor -> preview | {"protocol":1...     │
│ [Select Ent] │ preview -> editor | seq=1 channel=Ready  │
│ [Ping]       │ ...                                       │
│ [Stop]       │                                           │
│              │                                           │
│ Última linha │                                           │
│ enviada      │                                           │
│              │                                           │
└──────────────┴───────────────────────────────────────────┘
```

---

#### **Não Implementado** ❌

| Módulo | Complexidade | Prioridade |
|--------|--------------|------------|
| Spawn real do preview | Média | **P0** |
| IPC real (não mock) | Média | **P0** |
| Viewport embed (render preview) | Alta | **P0** |
| Scene tree panel | Média | **P1** |
| Inspector de entidades | Alta | **P1** |
| Asset browser | Média | **P1** |
| Undo/redo system | Alta | **P2** |
| Flow editor (grafos visuais) | Muito Alta | **P3** |
| Docking system | Baixa (crate) | **P1** |
| Hot reload de assets | Média | **P2** |

---

## Próximos Passos (Priorizados)

### **Fase 1: Bridge Funcional** (2-3 semanas)

| Tarefa | Descrição | Dependências |
|--------|-----------|--------------|
| **1.1** | Spawn real do processo preview | IPC mock pronto |
| **1.2** | IPC real via stdio (stdin/stdout) | 1.1 |
| **1.3** | Preview process em Zenith+Borealis | Borealis backend funcionando |
| **1.4** | Handshake completo (hello → ready) | 1.2 + 1.3 |

**Critério de aceite**: Editor abre, inicia preview, handshake funciona, transcript mostra mensagens reais

---

### **Fase 2: Editor Básico** (4-6 semanas)

| Tarefa | Descrição | Dependências |
|--------|-----------|--------------|
| **2.1** | Viewport embed (texture do preview) | 1.4 |
| **2.2** | Scene tree panel (hierarquia) | Borealis entities API |
| **2.3** | Inspector básico (propriedades) | 2.2 |
| **2.4** | Asset browser (scan de arquivos) | `walkdir`, `rfd` |
| **2.5** | Docking (egui_dock) | 2.1-2.4 |

**Critério de aceite**: Editor carrega cena, mostra entities, permite editar propriedades, preview renderiza no viewport

---

### **Fase 3: Editor Produtivo** (8-12 semanas)

| Tarefa | Descrição | Dependências |
|--------|-----------|--------------|
| **3.1** | Undo/redo system (command pattern) | 2.1-2.5 |
| **3.2** | Hot reload de assets | `notify`, 2.4 |
| **3.3** | Gizmos 3D (transform) | `transform-gizmo` crate |
| **3.4** | Flow editor (grafos visuais) | `petgraph`, 2.5 |
| **3.5** | Play mode com pause/step | 1.4, 3.1 |

**Critério de aceite**: Editor permite criar/editar cenas com undo, assets recarregam sozinhos, Flow editor funcional

---

## Riscos e Mitigações

### **Risco #1: Backend C Imaturo**
**Problema**: Structs tipadas e optionals ainda bloqueiam cenários Borealis

**Mitigação**:
- Usar fallback stub durante desenvolvimento do editor
- Priorizar fix do backend C em paralelo
- Testar editor com exemplos 2D simples primeiro

---

### **Risco #2: IPC Complexidade**
**Problema**: JSONL é legível mas pode ter overhead para frames/stats

**Mitigação**:
- Manter JSONL para commands/events (baixa frequência)
- Adicionar binary channel para frame data se necessário
- Benchmark cedo (Fase 2)

---

### **Risco #3: Editor-Preview Sync**
**Problema**: Manter estados sincronizados sem corromper authoring data

**Mitigação**:
- Play mode copia dados (não compartilha)
- Stop play mode restaura snapshot
- Editor metadata em arquivos separados (.editor.json)

---

### **Risco #4: Escopo 3D**
**Problema**: 18 módulos 3D = dobro de superfície para editor

**Mitigação**:
- Focar editor 2D primeiro
- Viewport 3D usa mesma infra (OpenGL via Raylib)
- Inspector 3D = extensões do 2D (Vector3 vs Vector2)

---

## Matriz de Dependências Críticas

```
Zenith Backend C ─────┬─────────> Borealis Runtime ─────> Editor Preview
                      │                                        │
                      └─────────> Borealis Scaffolds ──────────┘
```

**Gargalo atual**: Backend C (structs tipadas, optionals)

**Impacto no editor**:
- Sem backend C maduro → preview não roda exemplos complexos
- Sem preview real → editor IPC fica no mock
- **Solução**: Desenvolver editor com stub + exemplos mínimos

---

## Cronograma Estimado

| Fase | Duração | Entrega |
|------|---------|---------|
| **Fase 1** (Bridge) | 2-3 semanas | IPC real funcionando |
| **Fase 2** (Básico) | 4-6 semanas | Viewport + scene tree + inspector |
| **Fase 3** (Produtivo) | 8-12 semanas | Undo/redo + hot reload + Flow |
| **Total** | **3-5 meses** | Editor MVP utilizável |

**Dependência crítica**: Backend C deve estar maduro em 4-6 semanas para não bloquear Fase 2

---

## Referências Documentais

### **Decisions Borealis**
| # | Título | Status |
|---|--------|--------|
| 005 | Editor Ready Architecture | ✅ Accepted |
| 006 | Borealis Stack | ✅ Accepted |
| 007 | Borealis Flow | ✅ Accepted |
| 009 | 3D Performance Phasing | 📐 Proposed |
| 010 | 3D Canonical Order | 📐 Proposed |
| 011 | Editor Stack | ✅ Accepted |
| 012 | Editor Crates | ✅ Accepted |
| 013 | Editor Architecture | ✅ Accepted |
| 014 | Editor Preview IPC | ✅ Accepted |

### **Arquivos Chave**
```
packages/borealis/
├── architecture-summary.md
├── architecture-3d-summary.md
├── api-baseline-v1.md
├── backend-desktop-linker-profile-v1.md
├── known-limits-v1.md
├── migration-guide-v1.md
├── zpm-prep-v1.md
├── risks.md
└── decisions/
    ├── 005-editor-ready-architecture.md
    ├── 006-borealis-stack.md
    ├── 007-borealis-flow.md
    ├── 009-borealis-3d-performance-phasing.md
    ├── 011-borealis-editor-stack.md
    ├── 012-borealis-editor-crates.md
    ├── 013-borealis-editor-architecture.md
    ├── 014-borealis-editor-preview-ipc.md
    └── modules3d/ (18 files)
```

---

## Conclusão

O **borealis-editor** está **bem posicionado** architecturalmente:

✅ **Pontos fortes**:
- Decisões de arquitetura bem documentadas (14 decisions)
- IPC protocol já definido e mockado
- Stack Rust+egui é produtiva e adequada
- Borealis package é editor-ready por design
- Zenith language estável o suficiente para tooling

⚠️ **Atenção necessária**:
- Backend C é o gargalo principal
- Fase 1 (IPC real) é crítica para validar arquitetura
- Não pular para features avançadas (Flow) antes do básico funcionar

**Recomendação**: Focar em **Fase 1** nas próximas 2-3 semanas para ter IPC real rodando. Isso valida a arquitetura e dá confiança para investir nas fases seguintes.
