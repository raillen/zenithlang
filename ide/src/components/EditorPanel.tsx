import { useEffect, useState } from 'react';
import { Editor, loader } from '@monaco-editor/react';
import * as monaco from 'monaco-editor';
import { safeInvoke as invoke } from '../utils/tauri';
import { zenithLanguage, zenithConfig } from '../languages/zenith';

import { useProjectStore } from '../store/useProjectStore';

// Configurar o loader para usar a instância local do monaco-editor
loader.config({ monaco });

// Registrar a linguagem Zenith no carregamento inicial do Monaco
loader.init().then((monacoInstance) => {
  monacoInstance.languages.register({ id: 'zenith' });
  monacoInstance.languages.setMonarchTokensProvider('zenith', zenithLanguage);
  monacoInstance.languages.setLanguageConfiguration('zenith', zenithConfig);

  // Definir temas customizados
  const themes = {
    'zenith': { base: 'vs-dark', back: '#1e1e1e00' },
    'nord': { base: 'vs-dark', back: '#3b425200' },
    'neon': { base: 'vs-dark', back: '#0f081700' },
    'dracula': { base: 'vs-dark', back: '#282a3600' },
    'neuro': { base: 'vs', back: '#fdfdfd00' },
  };

  Object.entries(themes).forEach(([name, config]) => {
    monaco.editor.defineTheme(`xcode-${name}`, {
      base: config.base as any,
      inherit: true,
      rules: [
        { token: 'keyword', foreground: name === 'neuro' ? '2e8b57' : 'D783FF', fontStyle: 'bold' },
        { token: 'keyword.type', foreground: '5EB9FF' },
        { token: 'comment', foreground: '6C7986', fontStyle: 'italic' },
        { token: 'string', foreground: 'FC6A5D' },
        { token: 'number', foreground: 'D0BF69' },
        { token: 'type.identifier', foreground: 'A9E7FF' },
      ],
      colors: {
        'editor.background': config.back,
        'editor.foreground': name === 'neuro' ? '#333333' : '#FFFFFF',
        'editorLineNumber.foreground': '#5A5A5A',
        'editor.lineHighlightBackground': name === 'neuro' ? '#eeeeee' : '#2c2c2c',
      }
    });
  });
});

interface EditorPanelProps {
  path: string;
}

export function EditorPanel({ path }: EditorPanelProps) {
  const { theme } = useProjectStore();
  const [content, setContent] = useState<string>('');
  const [isLoading, setIsLoading] = useState(true);

  useEffect(() => {
    const loadFile = async () => {
      setIsLoading(true);
      try {
        const text = await invoke<string>('read_file', { path });
        setContent(text);
      } catch (err) {
        console.error("Error reading file:", err);
        setContent(`// Error loading file: ${err}`);
      } finally {
        setIsLoading(false);
      }
    };
    loadFile();
  }, [path]);

  const getLanguage = (fileName: string) => {
    if (fileName.endsWith('.zt')) return 'zenith';
    if (fileName.endsWith('.ts') || fileName.endsWith('.tsx')) return 'typescript';
    if (fileName.endsWith('.js') || fileName.endsWith('.jsx')) return 'javascript';
    if (fileName.endsWith('.json')) return 'json';
    if (fileName.endsWith('.html')) return 'html';
    if (fileName.endsWith('.css')) return 'css';
    if (fileName.endsWith('.md')) return 'markdown';
    return 'plaintext';
  };

  if (isLoading) {
    return (
      <div className="flex-1 flex items-center justify-center bg-black/5">
        <div className="text-white/20 animate-pulse text-sm">Loading {path.split(/[/\\]/).pop()}...</div>
      </div>
    );
  }

  return (
    <div className="flex-1 overflow-hidden h-full">
      <Editor
        height="100%"
        path={path}
        defaultLanguage={getLanguage(path)}
        value={content}
        theme={`xcode-${theme}`}
        options={{
          minimap: { enabled: true, side: 'right', scale: 0.7 },
          fontSize: 13,
          fontFamily: 'SF Mono, JetBrains Mono, monospace',
          lineHeight: 20,
          padding: { top: 10, bottom: 10 },
          scrollBeyondLastLine: false,
          automaticLayout: true,
          fontLigatures: true,
          cursorSmoothCaretAnimation: 'on',
          smoothScrolling: true,
          roundedSelection: true,
        }}
      />
    </div>
  );
}
