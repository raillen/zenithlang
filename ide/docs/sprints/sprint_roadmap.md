# Zenith Keter: Roadmap de Sprints

Este documento define a estratégia de desenvolvimento incremental da Zenith Keter, organizada em 7 sprints (2-3 dias cada) focadas em entregar valor funcional e estabilidade em cada etapa.

---

## 🏁 Sprint 0: Foundation & Setup
**Objetivo**: Estabelecer a infraestrutura básica e a janela "vibrante" da IDE.
- [ ] Inicialização do `Tauri v2` com React (Vite).
- [ ] Configuração do `Tailwind CSS` e carregamento inicial de fontes (Inter/SF Pro).
- [ ] Implementação de **Janela Sem Bordas (Frameless)** com efeitos de transparência (Acrylic/Mica).
- [ ] Configuração do estado global básico com `Zustand`.

## 📂 Sprint 1: Layout & Navigation
**Objetivo**: Criar a estrutura física da IDE e o sistema de arquivos.
- [ ] Implementação do `react-resizable-panels` para dividir Navigator, Editor e Inspector.
- [ ] Desenvolvimento do **Project Navigator** (File Explorer) com leitura real de diretórios via Tauri.
- [ ] Sistema de abas no estilo Xcode com suporte a arrastar e fechar.
- [ ] Mockup dos painéis laterais (Inspector e Symbols).

## ✍️ Sprint 2: The Core Editor (Monaco)
**Objetivo**: Transformar a IDE em um editor de código funcional.
- [ ] Integração do **Monaco Editor**.
- [ ] Registro da linguagem **Zenith** (Monarch Grammar) com highlight sintático básico.
- [ ] Sincronização de abertura/fechamento de arquivos entre o Navigator e o Editor.
- [ ] Persistência de arquivos no disco (Save/Save As).

## 🎨 Sprint 3: UI Aesthetics & Accessibility
**Objetivo**: Aplicar o "polimento Apple" e os recursos de neuro-acessibilidade.
- [ ] Implementação do sistema de temas (Zenith, Nord, Dracula, etc) usando classes Tailwind.
- [ ] Desenvolvimento do tema **Zenith Neuro (Focus)** focado em TDAH/Dislexia.
- [ ] Alternador dinâmico de fontes (SF Mono vs OpenDyslexic).
- [ ] Estilização das barras de scroll e botões para parecerem nativos do macOS.

## 🔍 Sprint 4: Omni-Search & Settings
**Objetivo**: Adicionar busca global e customização avançada.
- [ ] Implementação do `cmdk` para o **Omni-Search** (Cmd/Ctrl + P).
- [ ] Desenvolvimento do **Settings Modal** (Radix UI) com categorias estilo macOS.
- [ ] Integração do `i18next` para suporte a múltiplos idiomas na UI.
- [ ] Persistência de todas as preferências do usuário no disco.

## 🚀 Sprint 5: Zenith Build & Preview
**Objetivo**: Integrar o compilador Zenith e as ferramentas de visualização.
- [ ] Ponte Tauri IPC para executar o `ztc.lua` (Compiler Bridge).
- [ ] Barra de **Status Pill** dinâmica com progresso de compilação.
- [ ] Implementação do **Split View** para Markdown Preview e Web Preview.
- [ ] Consola integrada (`xterm.js`) para logs de erro e saída de execução.

## ✨ Sprint 6: Motion & Polish
**Objetivo**: Adicionar fluidez com animações e otimização final.
- [ ] Adição de animações de layout suaves com `motion-react`.
- [ ] Transições de "Focus Mode" e animações de modais.
- [ ] Auditoria final de performance e bundle size.
- [ ] Guia de uso final (`usage_guide.md`).

---

**Status Atual**: Pronto para iniciar a **Sprint 0**.
