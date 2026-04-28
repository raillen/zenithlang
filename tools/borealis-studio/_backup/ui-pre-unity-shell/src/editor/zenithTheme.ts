import { EditorView } from "@codemirror/view";
import { HighlightStyle, syntaxHighlighting } from "@codemirror/language";
import { tags } from "@lezer/highlight";

const borealisColors = {
  bg: "#1c1f26",
  fg: "#d4d4d8",
  comment: "#6b7280",
  keyword: "#c084fc",
  string: "#86efac",
  number: "#fbbf24",
  type: "#67e8f9",
  builtin: "#93c5fd",
  operator: "#9ca3af",
  meta: "#f472b6",
  selection: "#3b4252",
  cursor: "#e2e8f0",
  gutterBg: "#181b22",
  gutterFg: "#4b5563",
  activeLine: "#222630",
  matchBracket: "#3b4252",
};

export const borealisEditorTheme = EditorView.theme(
  {
    "&": {
      color: borealisColors.fg,
      backgroundColor: borealisColors.bg,
      fontSize: "13px",
      fontFamily: "'JetBrains Mono', 'Fira Code', 'SF Mono', monospace",
    },
    ".cm-content": {
      caretColor: borealisColors.cursor,
      padding: "8px 0",
    },
    ".cm-cursor, .cm-dropCursor": {
      borderLeftColor: borealisColors.cursor,
      borderLeftWidth: "2px",
    },
    "&.cm-focused .cm-selectionBackground, .cm-selectionBackground, .cm-content ::selection": {
      backgroundColor: borealisColors.selection,
    },
    ".cm-activeLine": {
      backgroundColor: borealisColors.activeLine,
    },
    ".cm-gutters": {
      backgroundColor: borealisColors.gutterBg,
      color: borealisColors.gutterFg,
      border: "none",
      borderRight: "1px solid #282c34",
      minWidth: "40px",
    },
    ".cm-activeLineGutter": {
      backgroundColor: borealisColors.activeLine,
      color: borealisColors.fg,
    },
    "&.cm-focused .cm-matchingBracket": {
      backgroundColor: borealisColors.matchBracket,
      outline: "1px solid #6b7280",
    },
    ".cm-searchMatch": {
      backgroundColor: "#3b4252",
      outline: "1px solid #4b5563",
    },
    ".cm-searchMatch.cm-searchMatch-selected": {
      backgroundColor: "#4b5563",
    },
    ".cm-tooltip": {
      backgroundColor: "#1e2028",
      border: "1px solid #333640",
      borderRadius: "4px",
    },
    ".cm-tooltip.cm-tooltip-autocomplete > ul > li": {
      padding: "2px 8px",
    },
    ".cm-tooltip-autocomplete ul li[aria-selected]": {
      backgroundColor: "#3b4252",
      color: borealisColors.fg,
    },
  },
  { dark: true },
);

const borealisHighlighting = HighlightStyle.define([
  { tag: tags.keyword, color: borealisColors.keyword },
  { tag: tags.comment, color: borealisColors.comment, fontStyle: "italic" },
  { tag: tags.blockComment, color: borealisColors.comment, fontStyle: "italic" },
  { tag: tags.lineComment, color: borealisColors.comment, fontStyle: "italic" },
  { tag: tags.string, color: borealisColors.string },
  { tag: tags.number, color: borealisColors.number },
  { tag: tags.typeName, color: borealisColors.type },
  { tag: tags.variableName, color: borealisColors.fg },
  { tag: [tags.standard(tags.variableName)], color: borealisColors.builtin },
  { tag: tags.operator, color: borealisColors.operator },
  { tag: tags.meta, color: borealisColors.meta },
  { tag: tags.bool, color: borealisColors.number },
  { tag: tags.null, color: borealisColors.number },
  { tag: tags.propertyName, color: borealisColors.fg },
  { tag: tags.function(tags.variableName), color: borealisColors.builtin },
  { tag: tags.definition(tags.variableName), color: "#e2e8f0" },
]);

export const borealisSyntaxHighlighting = syntaxHighlighting(borealisHighlighting);
