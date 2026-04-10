import { ZenithTheme } from "../themes";

/**
 * Applies the UI properties of a ZenithTheme to the document root,
 * overriding CSS custom properties to dynamically style Tailwind CSS.
 */
export function applyThemeToDOM(theme: ZenithTheme) {
  const root = document.documentElement;
  
  // Set the structural data attributes
  root.setAttribute('data-zenith-theme', theme.type);
  root.setAttribute('data-zenith-glass', theme.ui.glass.toString());
  
  // Apply the CSS Variables mapped to the tailwind config
  root.style.setProperty('--z-bg', theme.ui.background);
  root.style.setProperty('--z-panel', theme.ui.panel);
  root.style.setProperty('--z-border', theme.ui.border);
  root.style.setProperty('--z-text-main', theme.ui.textMain);
  root.style.setProperty('--z-text-muted', theme.ui.textMuted);
  root.style.setProperty('--z-primary', theme.ui.primary);
}

/**
 * Converts a simple ZenithTheme into a complex Monaco Editor theme.
 */
export function getMonacoTheme(theme: ZenithTheme): any {
  return {
    base: theme.type === 'dark' ? 'vs-dark' : 'vs',
    inherit: true,
    rules: [
      { token: 'keyword', foreground: theme.syntax.keyword },
      { token: 'string', foreground: theme.syntax.string },
      { token: 'identifier.function', foreground: theme.syntax.function },
      { token: 'number', foreground: theme.syntax.number },
      { token: 'type', foreground: theme.syntax.type },
      { token: 'comment', foreground: theme.syntax.comment, fontStyle: 'italic' },
      { token: 'operator.zenith', foreground: theme.ui.primary }, // Extra operator flair
    ],
    colors: {
      'editor.background': theme.ui.background,
      'editor.foreground': theme.ui.textMain,
      'editor.lineHighlightBackground': theme.type === 'dark' ? '#ffffff0a' : '#0000000a',
      'editorCursor.foreground': theme.ui.textMain,
      'editorIndentGuide.background': theme.ui.border,
      'editorSuggestWidget.background': theme.ui.panel,
      'editorSuggestWidget.border': theme.ui.border,
      'list.hoverBackground': theme.type === 'dark' ? '#ffffff10' : '#00000010',
    }
  };
}

/**
 * Converts a simple ZenithTheme into an xterm.js ITerminalOptions theme.
 */
export function getTerminalTheme(theme: ZenithTheme) {
  return {
    background: 'transparent', // TerminalPanel has its own background wrapper or uses panel bg
    foreground: theme.ui.textMain,
    cursor: theme.ui.textMain,
    selectionBackground: theme.type === 'dark' ? 'rgba(255, 255, 255, 0.2)' : 'rgba(0, 0, 0, 0.1)',
  };
}
