export type ZenithHubSectionId =
  | "overview"
  | "workflow"
  | "workspaces"
  | "themes"
  | "extensions"
  | "architecture"
  | "shortcuts"
  | "about";

export interface ZenithHubHighlight {
  label: string;
  value: string;
}

export interface ZenithHubBlock {
  title: string;
  body: string;
  bullets?: string[];
}

export interface ZenithHubSection {
  id: ZenithHubSectionId;
  navLabel: string;
  eyebrow: string;
  title: string;
  summary: string;
  highlights: ZenithHubHighlight[];
  blocks: ZenithHubBlock[];
}

export const ZENITH_HUB_SECTIONS: ZenithHubSection[] = [
  {
    id: "overview",
    navLabel: "Hub",
    eyebrow: "Zenith Hub",
    title: "Centro da IDE",
    summary:
      "O Hub substitui o menu legado da toolbar por um espaco de contexto, acoes rapidas e documentacao offline sem tirar o foco do workspace.",
    highlights: [
      { label: "Modo", value: "Interno e offline" },
      { label: "Entrada", value: "Toolbar ou Alt+Z" },
      { label: "Saida", value: "Browser opcional" },
    ],
    blocks: [
      {
        title: "Quando usar",
        body:
          "Abra o Hub para orientar a sessao atual, rever fluxos da IDE, acessar configuracoes de tema e extensoes ou descobrir atalhos sem sair do projeto.",
      },
      {
        title: "O que fica aqui",
        body:
          "Workspace atual, acessos principais, manual da IDE, guias de temas, guias de extensoes, arquitetura do projeto e uma secao dedicada de atalhos.",
      },
    ],
  },
  {
    id: "workflow",
    navLabel: "Usabilidade",
    eyebrow: "Fluxo",
    title: "Como a interface se organiza",
    summary:
      "A leitura da IDE segue uma hierarquia simples: navegacao na esquerda, contexto no topo, edicao no centro, inspecao na direita e feedback na base.",
    highlights: [
      { label: "Esquerda", value: "Explorer e contextos" },
      { label: "Centro", value: "Editor e quick open" },
      { label: "Direita", value: "Inspector" },
    ],
    blocks: [
      {
        title: "Toolbar hibrida",
        body:
          "A toolbar separa contexto e acao. O lado esquerdo fala de workspace, o centro de navegacao e o lado direito de execucao, diagnosticos e paines.",
      },
      {
        title: "Inspector",
        body:
          "O inspector nao replica a configuracao geral da IDE. Ele aponta para o toolchain do arquivo ativo, sinais do workspace e estado de runtime.",
      },
      {
        title: "Bottom panel",
        body:
          "Console, terminal e problems dividem o mesmo espaco para manter a leitura limpa. Use esse painel como zona de feedback e depuracao.",
      },
    ],
  },
  {
    id: "workspaces",
    navLabel: "Workspaces",
    eyebrow: "Projeto",
    title: "Workspaces, contextos e navegacao",
    summary:
      "O projeto ativo define a raiz do editor, do terminal, dos builds e da indexacao. Contextos salvos ajudam a restaurar sessoes recorrentes.",
    highlights: [
      { label: "Raiz", value: "Project root" },
      { label: "Contextos", value: "SQLite local" },
      { label: "Busca", value: "Quick Open" },
    ],
    blocks: [
      {
        title: "Troca de raiz",
        body:
          "Abrir pasta redefine a raiz do workspace e limpa o estado transitivo do editor. Terminais padrao e acoes de pipeline acompanham essa mudanca.",
      },
      {
        title: "Contextos salvos",
        body:
          "Os contextos guardam o arranjo da sessao: arquivos abertos, paineis, split mode, inspector e outras escolhas de workbench.",
      },
      {
        title: "Quick Open",
        body:
          "Use Quick Open para ir direto a arquivos pelo nome. Para comandos, abra a command palette em modo de comando.",
        bullets: [
          "Ctrl+P abre busca de arquivos",
          "Ctrl+Shift+P abre comandos",
          "Alt+Z abre este Hub",
        ],
      },
    ],
  },
  {
    id: "themes",
    navLabel: "Temas",
    eyebrow: "Aparencia",
    title: "Temas e personalizacao visual",
    summary:
      "A aparencia da IDE depende de tokens de tema, escala de interface e modulos de toolbar. O objetivo e manter clareza sem ruir a identidade visual.",
    highlights: [
      { label: "Base", value: "Tokens de UI" },
      { label: "Escala", value: "UI scale" },
      { label: "Toolbar", value: "Modulos configuraveis" },
    ],
    blocks: [
      {
        title: "Tema ativo",
        body:
          "Os temas controlam fundo, painel, borda, texto e cor primaria. A recomendacao e manter contraste consistente e usar uma cor de acento dominante.",
      },
      {
        title: "Escala da interface",
        body:
          "Use UI scale para ajustar legibilidade sem alterar o desenho estrutural da IDE. Isso ajuda em setups com telas densas ou menores.",
      },
      {
        title: "Toolbar modular",
        body:
          "A barra superior pode ocultar modulos secundarios como branch git, quick open, contadores, toggles de painel e controles de layout.",
      },
    ],
  },
  {
    id: "extensions",
    navLabel: "Extensoes",
    eyebrow: "Toolchains",
    title: "Extensoes, runtimes e LSP",
    summary:
      "As extensoes da IDE sao orientadas por perfis de runtime. Cada perfil pode definir binario, build, LSP e marcadores de workspace.",
    highlights: [
      { label: "Compiler", value: "Path configuravel" },
      { label: "LSP", value: "Por runtime" },
      { label: "Build", value: "Task por perfil" },
    ],
    blocks: [
      {
        title: "O que configurar",
        body:
          "Cada runtime pode apontar para compilador, servidor de linguagem e comando de build. O inspector mostra esses campos no contexto do arquivo ativo.",
      },
      {
        title: "Como estender",
        body:
          "Novos runtimes entram pelo mapa de perfis, marcadores de workspace e alvos de pipeline. O ideal e manter o desenho declarativo para reduzir logica espalhada.",
      },
      {
        title: "Ponto de entrada",
        body:
          "Quando quiser ajustar um toolchain, use o botao do inspector ou a aba Extensions nas preferencias. Esse fluxo e separado da configuracao geral da IDE.",
      },
    ],
  },
  {
    id: "architecture",
    navLabel: "Arquitetura",
    eyebrow: "Projeto",
    title: "Como a IDE foi montada",
    summary:
      "A IDE combina React no front, Zustand para estado local e Tauri para IO do sistema, terminal, indexacao e integracoes de runtime.",
    highlights: [
      { label: "Frontend", value: "React + Zustand" },
      { label: "Desktop", value: "Tauri" },
      { label: "Editor", value: "Monaco" },
    ],
    blocks: [
      {
        title: "Stores principais",
        body:
          "O estado do workbench vive principalmente em stores de workspace, comando e execucao. Esse arranjo deixa paines e modais sincronizados sem prop drilling pesado.",
      },
      {
        title: "Backend desktop",
        body:
          "Operacoes como abrir pasta, ler arquivos, criar terminais, buscar branch git e indexar o workspace sao resolvidas por comandos do Tauri.",
      },
      {
        title: "Criterio de design",
        body:
          "A linguagem visual atual busca um editor limpo, editorial e funcional. Menos ornemento, mais hierarquia espacial e mais contexto onde ele realmente ajuda.",
      },
    ],
  },
  {
    id: "shortcuts",
    navLabel: "Atalhos",
    eyebrow: "Produtividade",
    title: "Atalhos essenciais",
    summary:
      "Os atalhos aceleram navegacao, execucao e foco. Eles tambem documentam a filosofia da IDE: contexto rapido e poucas mudancas de modo.",
    highlights: [
      { label: "Hub", value: "Alt+Z" },
      { label: "Quick Open", value: "Ctrl+P" },
      { label: "Commands", value: "Ctrl+Shift+P" },
    ],
    blocks: [
      {
        title: "Workbench",
        body: "Atalhos mais usados para navegar a interface.",
        bullets: [
          "Ctrl+B alterna a sidebar",
          "Ctrl+J alterna o painel inferior",
          "Ctrl+Alt+I alterna o inspector",
          "Alt+1 e Alt+2 mudam o foco entre panes",
        ],
      },
      {
        title: "Execucao",
        body: "Fluxos de run e build ficam perto da toolbar e da palette.",
        bullets: [
          "F5 roda o alvo ativo",
          "Shift+F5 interrompe a tarefa ativa",
          "Ctrl+Shift+B dispara build",
        ],
      },
    ],
  },
  {
    id: "about",
    navLabel: "Sobre",
    eyebrow: "Produto",
    title: "Sobre o Zenith Keter",
    summary:
      "Zenith Keter esta se consolidando como uma IDE dedicada, com workflow proprio, runtime awareness e uma linguagem visual menos herdada de modelos genericos.",
    highlights: [
      { label: "Versao", value: "0.2 alpha" },
      { label: "Foco", value: "Workspace first" },
      { label: "Leitura", value: "Calma e objetiva" },
    ],
    blocks: [
      {
        title: "Objetivo",
        body:
          "Dar ao ecossistema Zenith uma superficie de trabalho propria, que trate editor, toolchain, historico, themes e extensoes como partes do mesmo sistema.",
      },
      {
        title: "Evolucao esperada",
        body:
          "Os proximos refinamentos naturais sao consolidacao do Hub, expansao do manual offline, melhoria do sistema de extensoes e mais polimento visual nos paineis.",
      },
    ],
  },
];

export function getZenithHubSection(sectionId: ZenithHubSectionId) {
  return (
    ZENITH_HUB_SECTIONS.find((section) => section.id === sectionId) ??
    ZENITH_HUB_SECTIONS[0]
  );
}

export function buildZenithHubBrowserDocument(activeSectionId: ZenithHubSectionId) {
  const htmlSections = ZENITH_HUB_SECTIONS.map((section) => {
    const highlightHtml = section.highlights
      .map(
        (highlight) => `
          <div class="metric">
            <div class="metric-label">${escapeHtml(highlight.label)}</div>
            <div class="metric-value">${escapeHtml(highlight.value)}</div>
          </div>
        `
      )
      .join("");

    const blockHtml = section.blocks
      .map((block) => {
        const bullets = block.bullets?.length
          ? `<ul>${block.bullets.map((item) => `<li>${escapeHtml(item)}</li>`).join("")}</ul>`
          : "";

        return `
          <section class="block">
            <h3>${escapeHtml(block.title)}</h3>
            <p>${escapeHtml(block.body)}</p>
            ${bullets}
          </section>
        `;
      })
      .join("");

    return `
      <article id="${section.id}" class="doc-section">
        <div class="eyebrow">${escapeHtml(section.eyebrow)}</div>
        <h2>${escapeHtml(section.title)}</h2>
        <p class="summary">${escapeHtml(section.summary)}</p>
        <div class="metrics">${highlightHtml}</div>
        <div class="blocks">${blockHtml}</div>
      </article>
    `;
  }).join("");

  const navHtml = ZENITH_HUB_SECTIONS.map(
    (section) => `
      <a href="#${section.id}" class="nav-link ${section.id === activeSectionId ? "is-active" : ""}">
        ${escapeHtml(section.navLabel)}
      </a>
    `
  ).join("");

  return `<!doctype html>
<html lang="en">
  <head>
    <meta charset="utf-8" />
    <meta name="viewport" content="width=device-width, initial-scale=1" />
    <title>Zenith Hub Manual</title>
    <style>
      :root {
        color-scheme: light;
        --bg: #f6f7f8;
        --panel: rgba(236, 238, 238, 0.82);
        --panel-strong: #ffffff;
        --border: rgba(0, 0, 0, 0.08);
        --text: #1d1d1f;
        --text-dim: #6e6e73;
        --accent: #0071e3;
      }
      * { box-sizing: border-box; }
      html, body { margin: 0; padding: 0; background: var(--bg); color: var(--text); font-family: Inter, system-ui, sans-serif; }
      body { min-height: 100vh; }
      .shell { display: grid; grid-template-columns: 220px minmax(0, 1fr); min-height: 100vh; }
      .rail {
        position: sticky;
        top: 0;
        align-self: start;
        display: flex;
        flex-direction: column;
        gap: 20px;
        min-height: 100vh;
        padding: 28px 20px;
        border-right: 1px solid var(--border);
        background: var(--panel);
        backdrop-filter: blur(16px);
      }
      .brand { font-size: 12px; font-weight: 800; letter-spacing: 0.24em; text-transform: uppercase; color: var(--text-dim); }
      .rail h1 { margin: 0; font-size: 22px; line-height: 1; letter-spacing: -0.03em; }
      .rail p { margin: 0; font-size: 13px; line-height: 1.6; color: var(--text-dim); }
      .nav { display: flex; flex-direction: column; gap: 6px; }
      .nav-link {
        display: block;
        padding: 10px 12px;
        border: 1px solid transparent;
        border-radius: 8px;
        color: var(--text-dim);
        font-size: 12px;
        font-weight: 700;
        letter-spacing: 0.08em;
        text-decoration: none;
        text-transform: uppercase;
      }
      .nav-link:hover, .nav-link.is-active {
        border-color: rgba(0, 113, 227, 0.14);
        background: rgba(0, 113, 227, 0.08);
        color: var(--accent);
      }
      main { padding: 32px; }
      .topline { display: flex; justify-content: space-between; align-items: center; gap: 16px; margin-bottom: 24px; }
      .topline .meta { font-size: 11px; color: var(--text-dim); text-transform: uppercase; letter-spacing: 0.18em; }
      .doc-section { padding: 28px 0; border-bottom: 1px solid var(--border); }
      .doc-section:last-child { border-bottom: 0; }
      .eyebrow { margin-bottom: 8px; font-size: 11px; font-weight: 800; letter-spacing: 0.18em; text-transform: uppercase; color: var(--accent); }
      .doc-section h2 { margin: 0; font-size: clamp(28px, 4vw, 42px); line-height: 1; letter-spacing: -0.04em; }
      .summary { max-width: 70ch; margin: 14px 0 0; color: var(--text-dim); font-size: 15px; line-height: 1.7; }
      .metrics { display: grid; grid-template-columns: repeat(auto-fit, minmax(160px, 1fr)); gap: 12px; margin-top: 20px; }
      .metric { padding: 14px; border: 1px solid var(--border); border-radius: 8px; background: var(--panel-strong); }
      .metric-label { font-size: 10px; font-weight: 800; text-transform: uppercase; letter-spacing: 0.14em; color: var(--text-dim); }
      .metric-value { margin-top: 4px; font-size: 15px; font-weight: 700; color: var(--text); }
      .blocks { display: grid; gap: 12px; margin-top: 22px; }
      .block { padding: 16px; border: 1px solid var(--border); border-radius: 8px; background: var(--panel-strong); }
      .block h3 { margin: 0 0 8px; font-size: 15px; }
      .block p { margin: 0; color: var(--text-dim); font-size: 14px; line-height: 1.7; }
      .block ul { margin: 12px 0 0 18px; color: var(--text-dim); font-size: 13px; line-height: 1.7; }
      @media (max-width: 960px) {
        .shell { grid-template-columns: 1fr; }
        .rail { position: static; min-height: auto; border-right: 0; border-bottom: 1px solid var(--border); }
        main { padding: 20px; }
      }
    </style>
  </head>
  <body>
    <div class="shell">
      <aside class="rail">
        <div>
          <div class="brand">Zenith Hub</div>
          <h1>Manual offline da IDE</h1>
          <p>Guia local de uso, temas, extensoes e arquitetura do workbench.</p>
        </div>
        <nav class="nav">${navHtml}</nav>
      </aside>
      <main>
        <div class="topline">
          <div class="meta">Zenith Keter / Manual offline</div>
          <div class="meta">Version 0.2 alpha</div>
        </div>
        ${htmlSections}
      </main>
    </div>
  </body>
</html>`;
}

function escapeHtml(value: string) {
  return value
    .replace(/&/g, "&amp;")
    .replace(/</g, "&lt;")
    .replace(/>/g, "&gt;")
    .replace(/"/g, "&quot;")
    .replace(/'/g, "&#39;");
}
