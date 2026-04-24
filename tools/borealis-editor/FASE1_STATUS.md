# Fase 1: Bridge Funcional - Status

**Data**: 2026-04-23  
**Status**: ✅ Concluída (E2E IPC funcional)

---

## Resumo Executivo

**Infrastructure IPC (Rust)**: ✅ 100%  
**Preview Runner (Zenith)**: ✅ 100% (compilando e respondendo IPC)  
**Mock Mode (Temporário)**: ✅ 100%  
**Handshake E2E Real**: ✅ 100%

---

## Tarefas Implementadas

### ✅ 1.1 Spawn real do processo preview
**Arquivo**: `src/ipc.rs`

**Implementação**: `resolve_preview_binary()` detecta automaticamente:
1. Binary compilado (`preview/build/borealis-preview.exe`)
2. Fallback para `zt.exe run preview/zenith.ztproj`

**Status**: Funcional

---

### ✅ 1.2 IPC real via stdio
**Mecanismo**: JSONL sobre stdin/stdout

**Thread de leitura**: Background thread + `mpsc::channel`
**Escrita**: `stdin` piped com flush por comando

**Status**: Funcional

---

### ✅ 1.3 Preview Runner em Zenith
**Arquivo**: `preview/preview_runner.zt`

**Implementação completa**:
- Loop principal com `io.read_line()` para ler stdin
- Parser de mensagens via `text.contains()` (JSON simples)
- Respostas JSONL via `io.write()`
- Mensagens suportadas:
  - `hello` → `hello` + `status: ready`
  - `enter_play_mode` → `status: playing`
  - `stop_play_mode` → `status: stopped`
  - `ping` → `pong` (com token echo)
  - `open_scene` → `status: loading` → `status: ready`
  - `open_project` → `status: loading` → `status: ready`

**Utilitários**:
- `build_envelope()` - monta envelope JSONL
- `extract_ping_token()` - extrai token do ping
- `int_to_text()` - converte int para text (Zenith não tem built-in)

**Status**: ✅ Compilando e testado

---

### ✅ 1.4 Mock Mode (Temporário)
**Arquivo**: `src/ipc.rs`

**Implementação**: `PreviewBridge` com flag `mock: bool` (default: `true`)

Quando mock está ativo:
- Não spawn processo
- Simula responses via `simulate_mock_response()`

**Status**: ✅ Funcional

---

### ✅ 1.5 UI com Mode Toggle
**Arquivo**: `src/app.rs`

- Top bar mostra indicador `🔧 MOCK` quando ativo
- Botão "Switch to Real Preview" / "Switch to Mock"
- Transcript registra mudanças de modo

---

## Estrutura de Arquivos

```
tools/borealis-editor/
├── src/
│   ├── main.rs
│   ├── app.rs (UI com mode toggle)
│   ├── ipc.rs (IPC real + mock + binary resolution)
│   └── messages.rs (protocolo v1)
├── preview/
│   ├── zenith.ztproj
│   ├── preview_runner.zt (IPC runner completo)
│   └── build/
│       └── borealis-preview.exe (binary compilado)
├── Cargo.toml
├── ANALISE_COMPLETA.md
└── FASE1_STATUS.md
```

---

## Como Rodar

### Editor Rust
```bash
cd tools/borealis-editor
cargo run
```

### Preview Runner (standalone test)
```bash
cd tools/borealis-editor
# Compilar
zt run preview/zenith.ztproj

# Testar IPC manualmente
echo '{"protocol":1,"seq":1,"channel":"command","kind":"hello","payload":{"role":"editor"}}' | preview/build/borealis-preview.exe
```

### Modos de Operação

**Mock Mode (default)**:
- Editor inicia em mock mode
- Preview responses são simuladas internamente
- Não requer binary compilado

**Real Mode**:
- Clique "Switch to Real Preview"
- Requer `preview/build/borealis-preview.exe` compilado
- IPC E2E real via stdio

---

## Critérios de Aceite

| Critério | Status | Notas |
|----------|--------|-------|
| Editor spawnar processo | ✅ | Binary resolution automático |
| IPC stdio encode/decode | ✅ | JSONL funcionando |
| Thread-safe (mpsc) | ✅ | Channel para leitura |
| Update por frame | ✅ | `preview.update()` no app |
| Preview runner compila | ✅ | Zenith → C → exe |
| Mock mode funcional | ✅ | Todas as responses simuladas |
| UI reflete estado | ✅ | State machine completa |
| Transcript logs | ✅ | Outgoing + incoming |
| Preview responde IPC real | ✅ | Testado via pipe |
| Handshake E2E real | ✅ | hello → ready funcionando |
| Play mode flow | ✅ | enter/stop → status |
| Ping/pong | ✅ | Token echo |

---

## Testes Manuais Realizados

### Mock Mode
```
Start Preview → status: Ready ✅
Enter Play Mode → status: Playing ✅
Stop Preview → status: Stopped ✅
Ping → Pong ✅
```

### Real Preview Runner (via pipe)
```bash
echo hello | borealis-preview.exe
→ {"protocol":1,"seq":1,"channel":"event","kind":"hello","payload":{"role":"preview","project_path":""}}
→ {"protocol":1,"seq":2,"channel":"event","kind":"status","payload":{"status":"ready"}}

echo hello + enter_play_mode + ping + stop_play_mode
→ status: starting
→ hello (preview)
→ status: ready
→ status: playing
→ pong (token echoed)
→ status: stopped
```

---

## Próximos Passos

### Fase 2: Viewport Embedding
1. Embed preview window no viewport
2. Render target sharing
3. Input forwarding (mouse/keyboard)

### Melhorias do Preview Runner
1. Parsing JSON real (quando stdlib json suportar nested)
2. Error handling com `channel: error`
3. Frame stats reporting
4. Entity selection sync

---

## Conclusão

**Fase 1: 100% concluída**

- ✅ Infrastructure Rust: sólida
- ✅ Protocolo IPC: definido e implementado
- ✅ Preview Runner Zenith: compilando e respondendo
- ✅ Mock mode: validação rápida
- ✅ E2E real: handshake e message flow funcionais

**O bridge editor ↔ preview está operacional em ambos os modos.**
