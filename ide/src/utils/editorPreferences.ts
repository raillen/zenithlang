export const editorFontOptions = [
  {
    id: 'SF Mono',
    label: 'SF Mono',
    stack: "'SF Mono', 'JetBrains Mono', 'Cascadia Code', monospace",
  },
  {
    id: 'JetBrains Mono',
    label: 'JetBrains Mono',
    stack: "'JetBrains Mono', 'Cascadia Code', 'SF Mono', monospace",
  },
  {
    id: 'Fira Code',
    label: 'Fira Code',
    stack: "'Fira Code', 'JetBrains Mono', 'Cascadia Code', monospace",
  },
  {
    id: 'Cascadia Code',
    label: 'Cascadia Code',
    stack: "'Cascadia Code', 'JetBrains Mono', 'SF Mono', monospace",
  },
  {
    id: 'IBM Plex Mono',
    label: 'IBM Plex Mono',
    stack: "'IBM Plex Mono', 'JetBrains Mono', monospace",
  },
] as const;

export type EditorFontFamily = (typeof editorFontOptions)[number]['id'];

export function getEditorFontStack(fontFamily: EditorFontFamily) {
  return editorFontOptions.find((option) => option.id === fontFamily)?.stack ?? editorFontOptions[0].stack;
}

export function getEditorLineHeight(fontSize: number) {
  return Math.max(18, Math.round(fontSize * 1.55));
}
