# Zenith Keter Implementation Plan

Este plano detalha a arquitetura, stack tecnológica e funcionalidades para a construção da nova IDE da linguagem Zenith, com foco em uma experiência premium inspirada no Xcode do macOS.

## User Review Required

> [!IMPORTANT]
> A stack recomendada foge do PySide (usado anteriormente) para adotar **Tauri + Monaco Editor**. Isso permite maior fidelidade visual ao macOS e suporte nativo a tecnologias web, mas requer conhecimento em Rust (backend) e React (frontend).
> 
> [!TIP]
> Dado que o usuário possui TDAH, a IDE será focada em baixo ruído visual, organização lógica e feedback instantâneo (vincular o compilador Zenith diretamente à UI).

---

## 🎨 Padrão de Design (Estética Xcode macOS)

O objetivo é capturar a essência do design da Apple (Human Interface Guidelines):

| Camada | Tecnologia | Justificativa |
| :--- | :--- | :--- |
| **Framework Base** | **Tauri (Rust)** | Extremamente leve e seguro. Efeitos de transparência nativos. |
| **Interface (UI)** | **React** | Componentização avançada para interfaces complexas. |
| **Editor de Código** | **Monaco Editor** | Motor robusto para edição e suporte a linguagens web. |
| **Layout/Split View** | `react-resizable-panels` | Gerenciamento de proporções e salvamento de estado de layout. |
| **Busca/Comandos** | `cmdk` | Paleta de comandos (Omni-Search) de alta performance. |
| **Componentes UI** | `radix-ui` | Primitivos acessíveis para Menus de Contexto e Modais. |
| **Animações** | **Motion-react** | Fluidez estilo Apple e mola física. |
| **Internacionalização** | `i18next` | Suporte nativo a múltiplos idiomas da interface. |
| **Estado/Persistência**| `zustand` + `persist`| Estado global com sincronização de preferências do usuário. |
| **Estilização** | **Tailwind CSS** | Design system Apple e tokens de design customizados. |
| **Ícones** | **Lucide + SF Symbols** | Ícones consistentes com o ecossistema macOS. |

1.  **Vibrancy & Translucency**: Uso de efeitos "Glassmorphism" nas barras laterais e barras de ferramentas (Acrylic no Windows).
2.  **Tipografia**:
    *   Interface: **SF Pro Display** (ou Inter como fallback).
    *   Editor: **SF Mono** (ou JetBrains Mono/Cascadia Code).
3.  **Ícones**: Uso exclusivo de **Lucide Icons** customizados para parecerem com os **SF Symbols** da Apple.
4.  **Minimalismo**: Componentes sem bordas pesadas, usando sombras sutis e variações de cinza para separação visual.
5.  **Animações Funcionais (Motion)**:
    *   Uso de molas (`spring`) em vez de easing linear.
    *   Transições de layout automáticas para abertura de painéis secundários.
    *   Escala sutil no surgimento de modais.
    *   **Diretrizes de Acessibilidade**: Respeitar a preferência do sistema `prefers-reduced-motion`, desativando animações complexas caso o usuário solicite.

---

## 🎨 Temas e Acessibilidade Cognitiva

A Zenith Keter será uma das primeiras focadas ativamente em neurodiversidade, oferecendo uma gama de temas estéticos e funcionais:

### 1. Lista de Temas Base
- **Zenith (Light/Dark)**: O tema oficial, otimizado para a identidade visual da linguagem.
- **Neon Lights**: Alto contraste com cores vibrantes sobre fundo escuro (estilo Synthwave).
- **No Happiness Here**: Tema monocromático/escala de cinza para foco absoluto e baixo estímulo.
- **Sublime (Light/Dark)**: Inspirado no clássico Sublime Text (Monokai-ish).
- **Nord**: Tons de azul glaciais, calmos e frios.
- **Dracula**: O classic e amado tema escuro de alta legibilidade.

### 2. Pesquisa e Diretrizes para Acessibilidade (TDAH e Dislexia)
Após pesquisa pedagógica, o tema **Zenith Neuro (Focus)** seguirá estas diretrizes:

#### 🧠 Para Dislexia (Foco em Legibilidade de Caracteres):
- **Cores de Fundo**: Evitar o branco puro (#FFF) ou preto puro (#000). Utilizaremos fundos "Creme" ou "Slate" suave para reduzir o efeito de "vibração" das letras.
- **Tipografia**: Suporte nativo à fonte **OpenDyslexic** ou **Atkinson Hyperlegible**. Estas fontes possuem bases mais pesadas, evitando que as letras "rodem" mentalmente.
- **Text Spacing**: Aumento automático do `line-height` (1.5x) e `letter-spacing` para evitar o efeito de "rios de branco" que atrapalham a leitura.

#### ⚡ Para TDAH (Foco em Gerenciamento de Atenção):
- **Paleta Mutada**: Cores de sintaxe que não sejam "gritantes". Tons pastéis ou dessaturados ajudam a manter a calma visual.
- **Active Line Highlighting**: O destaque da linha atual será nítido, enquanto o restante do código pode opcionalmente ser levemente obscurecido (Focus Mode).
- **Bracket Pair Colorization**: Uso de cores consistentes para pares de parênteses/chaves, facilitando o rastreio visual da estrutura lógica.
- **Redução de Ruído**: Capacidade de esconder todos os painéis e a barra de status com uma tecla (Zen Mode), deixando apenas o código.

---

## 🏛️ Descrição dos Painéis e Funções

A IDE será dividida em 4 áreas principais, seguindo o layout do Xcode 15/16:

### 1. Project Navigator (Painel Esquerdo)
- **File Explorer**: Gerenciamento de arquivos e pastas com ícones coloridos.
- **Search**: Busca global por texto em todo o projeto.
- **Symbol Navigator**: Lista de Traits, Structs e Funções no arquivo ativo (via análise Zenith).
- **Source Control**: Integração básica com Git.

### 2. Main Editor (Painel Central)
- **Abas Estilizadas**: Abas com cantos arredondados e ícones de tipo de arquivo.
- **Monaco Engine**: Suporte a Zenith, HTML, CSS, JS, TS, JSON, etc.
- **Split View**: Suporte a divisão vertical/horizontal via `react-resizable-panels`.
- **Markdown Mode**: Visualização renderizada em tempo real (lado a lado).
- **Breadcrumbs**: Caminho do arquivo (`src > core > utils.zt`) no topo do editor.

### 3. Inspector & Library (Painel Direito)
- **File Inspector**: Informações do arquivo, encoding, permissões.
- **Zenith Context**: Detalhes do módulo atual, imports e dependências.
- **Attribute Inspector**: Painel para edição visual de propriedades (útil para desenvolvimento Web).

### 4. Status Pill & Toolbar (Topo)
- **Status Central**: Uma "pílula" no estilo Xcode que mostra:
  - Progresso da compilação.
  - Número de erros/avisos.
  - "Building Zenith UI..."
- **Run/Stop Buttons**: Controle do ciclo de execução.
- **Target Selector**: Alternar entre rodar como Script Lua ou aplicação Web.

### 5. Settings Modal (Menu de Configurações)
- **Interface**: Modal centralizado com fundo desfoque (Blur Backdrop) e barra lateral de categorias estilo macOS.
- **Categorias e Opções**:
  - **Geral**: Idioma da IDE (i18n), verificações de atualização.
  - **Aparência**: Troca de temas (Zenith, Nord, etc.), opacidade da janela (Acrylic), ícones.
  - **Editor**: Font Family (SF Mono, OpenDyslexic, etc.), Font Size, Line Height, Ligatures.
  - **Keybinds**: Editor visual para remapeamento de teclas (JSON Editor como fallback).
  - **Runners**: Configuração de scripts de compilação, argumentos do `ztc.lua` e diretórios de saída.

---

## 🏛️ Arquitetura Técnica & Integração

### Backend (Rust / Tauri)
- **FileSystem Hub**: APIs nativas de alta performance para leitura/escrita.
- **ZTC Bridge**: Wrapper em Rust para chamar o compilador `ztc.lua` via linha de comando ou embutido.
- **LSP Host**: Servidor para prover autocompletar e linting para Zenith.

### Frontend (React + Tailwind)
- **State Management**: Zustand para gerenciar o estado global da IDE (arquivos abertos, tema).
- **Styling**: Tailwind CSS com tokens customizados (`bg-xcode-sidebar`, `text-apple-label`).
- **Tiling Engine**: Lógica de Split View que permite mover painéis livremente.

---

## 🚀 Implementação Detalhada das Novas Bibliotecas

### 1. Layout Dinâmico (`react-resizable-panels`)
- **Funcionalidade**: Sistema de "Panels" que permite ao usuário redimensionar o Project Navigator, o Inspector e o Console.
- **Implementação**:
  - Utilização de `PanelGroup` na raiz da aplicação.
  - Definição de `minSize` e `maxSize` para evitar que painéis quebrem a interface.
  - Uso do plugin de persistência para que a IDE lembre exatamente onde os painéis estavam na última sessão.

### 2. Omni-Search / Paleta de Comandos (`cmdk`)
- **Funcionalidade**: Um menu pop-up (Cmd+P) para busca rápida de arquivos, símbolos de código e execução de comandos da IDE.
- **Implementação**:
  - Gatilho global de teclado via hooks do React.
  - Indexação em tempo real do sistema de arquivos através de Rust (Tauri FS) para busca ultra-rápida.
  - Visual estilizado com `backdrop-filter: blur` para flutuar sobre o editor.

### 3. Primitivos Acessíveis (`radix-ui`)
- **Funcionalidade**: Menus de contexto (botão direito), Dropdowns (Toolbar) e o Modal de Configurações.
- **Implementação**:
  - **ContextMenu**: Fornecer ações rápidas (New File, Rename, Run) nos itens do File Explorer.
  - **Dialog**: Base para todos os modais da IDE, garantindo que o foco do teclado e leitores de tela funcionem corretamente.
  - Estilização via Tailwind para mascarar os componentes brutos como Elementos UI do macOS Big Sur/Sonoma.

### 4. Internacionalização e Sincronização (`i18next` & `zustand`)
- **i18next**: Gerenciamento de arquivos JSON de tradução (`pt-BR`, `en-US`). Troca dinâmica via Settings Modal sem recarregar a IDE.
- **Zustand Persist**: Implementação de uma camada de armazenamento que salva automaticamente as preferências do usuário (Ex: "Ativar Focus Mode", "Fonte OpenDyslexic", "Tema Nord") no sistema de arquivos local através do Tauri.

---

## 🛠️ Meta-Desenvolvimento: Zenith Keter Skills

Para garantir a qualidade do projeto, desenvolveremos 4 skills especializadas:

### 1. `zenith-ide-architect`
- **Conteúdo**: Guias para Tauri IPC, Monaco Language registration, e estrutura de componentes React.
- **Scripts**: Boilerplates para novos comandos Rust e componentes de interface.

### 2. `zenith-ide-ui-auditor`
- **Conteúdo**: Checklist de design Apple (Human Interface Guidelines) e diretrizes de neuro-acessibilidade.
- **Verificação**: Scripts de auditoria de CSS e contraste de cores.

### 3. `zenith-ide-tester`
- **Conteúdo**: Estratégias de teste para Tauri (backend em Rust) e Vitest/Playwright (frontend).
- **Scripts**: Mocks para sistema de arquivos e comandos do compilador Zenith.

### 4. `zenith-ide-performance`
- **Conteúdo**: Melhores práticas para redução de bundle size, otimização de renderização do Monaco e monitoramento de memória.
- **Análise**: Benchmarks de tempo de startup e resposta de UI.
