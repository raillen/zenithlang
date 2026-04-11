# 📋 Backlog e Pendências Futuras (Zenith Keter)

Este documento rastreia features arquitetadas, mas que exigem validações futuras, polimento nativo ou infraestrutura complementar.

## 🔍 Busca Global e Quick Open (Arquivos do Sistema)
**Status Técnico:** Backend Rust criado (`search_in_files` e `search_file_names`), React State vinculado.
**Pendência / Validação:**
- Acesso à File System local: Como a IDE roda sobre o Tauri, a varredura nativa pelo disco rígido só funcionará com build próprio (rodando em Janela Desktop via `npm run tauri dev`). 
- **Tarefa Futura 1:** Realizar os testes práticos de velocidade massiva na busca global com o Tauri nativo rodando em um projeto com centenas de códigos locais `.zt`.
- **Tarefa Futura 2 (Opcional):** Implementar um _Fallback_ temporário no Frontend em JavaScript para a Command Palette funcionar na Web com um mock de arquivos temporário.

## ⌨️ Sistema de Customização de Atalhos (Keybinds)
**Status Técnico:** Núcleo de Comandos criado (`useCommandStore.ts`) e dispatcher reativo inserido no `App.tsx`.
**Pendência / UI:**
- **Tarefa Futura:** Na tela de configurações (`SettingsDialog.tsx`), adicionar a Nova Aba "Keybinds". Essa aba será uma tabela iterativa visual mapendo a `keymap` gravada em SQLite. O usuário poderá clicar num comando (Ex: `zenith.workbench.action.quickOpen`) e pressionar uma nova combinação visual para reverter o atalho padrão.

## LSP (Language Server Protocol) e Inteligencia Semantica
- **Implementado (v1):** Monaco agora usa linguagem real por arquivo e registra autocomplete semantico local para Zenith, TypeScript/JavaScript, C#/.NET, Rust, Lua e Ruby.
- **Configuracao ativa:** O campo "LSP Server Path" da aba *Extensoes* foi liberado para cada runtime e ja aparece no contexto do provider semantico.
- **Proximo passo:** Implementar o bridge nativo JSON-RPC para iniciar servidores LSP externos, sincronizar documentos e consumir respostas reais de textDocument/completion.
