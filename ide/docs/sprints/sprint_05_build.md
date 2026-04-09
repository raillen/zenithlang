# Sprint 5: Zenith Build & Previews

**Foco**: Integração com o compilador e visualização de mídia.

### ✅ Entregáveis:
1. **Compiler Bridge**: Botão de Build que dispara o `ztc.lua` via Tauri Command.
2. **Status Pill**: Indicador central que mostra o sucesso ou erro da compilação.
3. **Markdown Previewer**: Modo split view para renderização de Markdown.
4. **Web Viewer**: WebView integrada para testes rápidos de HTML/CSS/JS.

### 🛠️ Tarefas Técnicas:
- Implementar chamada de processo externo no Rust (spawn process).
- Capturar `stdout` e `stderr` do compilador e exibir no log da IDE.
- Desenvolver o componente de visualização Markdown com sincronização de scroll.
- Integrar a WebView nativa do Tauri para o modo Web Preview.

---
*Anterior: [Sprint 4 - Search](./sprint_04_search.md)*
*Próxima Sprint: [Sprint 6 - Motion & Polish](./sprint_06_polish.md)*
