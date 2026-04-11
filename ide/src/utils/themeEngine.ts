import { ZenithTheme } from "../themes";

const MIN_UI_SCALE = 0.85;
const MAX_UI_SCALE = 1.35;

/**
 * Applies the UI properties of a ZenithTheme to the document root,
 * overriding CSS custom properties to dynamically style Tailwind CSS.
 */
export function applyThemeToDOM(theme: ZenithTheme) {
  const root = document.documentElement;

  // Set the structural data attributes
  root.setAttribute("data-zenith-theme", theme.type);
  root.setAttribute("data-zenith-glass", theme.ui.glass.toString());

  // Apply the CSS Variables mapped to the tailwind config
  root.style.setProperty("--z-bg", theme.ui.background);
  root.style.setProperty("--z-panel", theme.ui.panel);
  root.style.setProperty("--z-border", theme.ui.border);
  root.style.setProperty("--z-text-main", theme.ui.textMain);
  root.style.setProperty("--z-text-muted", theme.ui.textMuted);
  root.style.setProperty("--z-primary", theme.ui.primary);
}

export function clampUiScale(scale: number) {
  if (!Number.isFinite(scale)) return 1;
  return Math.min(MAX_UI_SCALE, Math.max(MIN_UI_SCALE, Number(scale)));
}

export function applyUiScaleToDOM(scale: number) {
  if (typeof document === "undefined") return;

  const normalized = clampUiScale(scale);
  const root = document.documentElement;
  const body = document.body;

  root.style.setProperty("--z-ui-scale", normalized.toFixed(2));

  if (!body) return;

  body.style.zoom = normalized.toString();
  body.style.width = `${100 / normalized}%`;
  body.style.minHeight = `${100 / normalized}%`;
  body.style.transformOrigin = "top left";
}

/**
 * Converts a simple ZenithTheme into a complex Monaco Editor theme.
 */
export function getMonacoTheme(theme: ZenithTheme): any {
  return {
    base: theme.type === "dark" ? "vs-dark" : "vs",
    inherit: true,
    rules: [
      { token: "keyword", foreground: theme.syntax.keyword },
      { token: "string", foreground: theme.syntax.string },
      { token: "identifier.function", foreground: theme.syntax.function },
      { token: "number", foreground: theme.syntax.number },
      { token: "type", foreground: theme.syntax.type },
      { token: "comment", foreground: theme.syntax.comment, fontStyle: "italic" },
      { token: "operator.zenith", foreground: theme.ui.primary },
    ],
    colors: {
      "editor.background": theme.ui.background,
      "editor.foreground": theme.ui.textMain,
      "editor.lineHighlightBackground": theme.type === "dark" ? "#ffffff0a" : "#0000000a",
      "editorCursor.foreground": theme.ui.textMain,
      "editorIndentGuide.background": theme.ui.border,
      "editorSuggestWidget.background": theme.ui.panel,
      "editorSuggestWidget.border": theme.ui.border,
      "list.hoverBackground": theme.type === "dark" ? "#ffffff10" : "#00000010",
    }
  };
}

/**
 * Converts a simple ZenithTheme into an xterm.js ITerminalOptions theme.
 */
export function getTerminalTheme(theme: ZenithTheme) {
  const isDark = theme.type === "dark";

  return {
    background: theme.ui.background,
    foreground: theme.ui.textMain,
    cursor: theme.ui.primary,
    cursorAccent: theme.ui.background,
    selectionBackground: isDark ? "rgba(255, 255, 255, 0.2)" : "rgba(0, 0, 0, 0.1)",

    // ANSI Colors derived from theme syntax
    // ANSI Colors - Curated for a premium, vibrant feel
    black: isDark ? "#1a1b26" : "#000000",
    red: "#f7768e",    // Soft vibrant red
    green: "#9ece6a",  // Fresh vibrant green
    yellow: "#e0af68", // Warm amber
    blue: "#7aa2f7",   // Sky blue
    magenta: "#bb9af7", // Elegant purple
    cyan: "#7dcfff",    // Soft cyan
    white: isDark ? "#a9b1d6" : "#444444",

    brightBlack: isDark ? "#414868" : "#999999",
    brightRed: "#ff9e64",    // Vibrant orange-red
    brightGreen: "#73daca",  // Teal-ish green
    brightYellow: "#e0af68", // Consistent amber
    brightBlue: "#b4f9f8",   // Very light cyan/blue
    brightMagenta: "#ff007c", // Hot pink/magenta
    brightCyan: "#2ac3de",   // Deep cyan
    brightWhite: isDark ? "#c0caf5" : "#000000",
  };
}
