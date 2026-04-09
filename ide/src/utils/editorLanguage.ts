export function getEditorLanguage(fileName: string) {
  if (fileName.endsWith('.zt')) return 'zenith';
  if (fileName.endsWith('.lua')) return 'lua';
  if (fileName.endsWith('.ts') || fileName.endsWith('.tsx')) return 'typescript';
  if (fileName.endsWith('.js') || fileName.endsWith('.jsx')) return 'javascript';
  if (fileName.endsWith('.json')) return 'json';
  if (fileName.endsWith('.html')) return 'html';
  if (fileName.endsWith('.css')) return 'css';
  if (fileName.endsWith('.md')) return 'markdown';
  return 'plaintext';
}
