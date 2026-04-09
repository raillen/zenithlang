import { loader } from '@monaco-editor/react';
import * as monaco from 'monaco-editor';
import editorWorker from 'monaco-editor/esm/vs/editor/editor.worker?worker';
import jsonWorker from 'monaco-editor/esm/vs/language/json/json.worker?worker';
import cssWorker from 'monaco-editor/esm/vs/language/css/css.worker?worker';
import htmlWorker from 'monaco-editor/esm/vs/language/html/html.worker?worker';
import tsWorker from 'monaco-editor/esm/vs/language/typescript/ts.worker?worker';
import { zenithLanguage, zenithConfig } from '../languages/zenith';

let monacoConfigured = false;

export function configureMonaco() {
  if (monacoConfigured) {
    return;
  }

  const getWorker = (_moduleId: string, label: string) => {
    switch (label) {
      case 'json':
        return new jsonWorker();
      case 'css':
      case 'scss':
      case 'less':
        return new cssWorker();
      case 'html':
      case 'handlebars':
      case 'razor':
        return new htmlWorker();
      case 'typescript':
      case 'javascript':
        return new tsWorker();
      default:
        return new editorWorker();
    }
  };

  (
    globalThis as typeof globalThis & {
      MonacoEnvironment?: {
        getWorker: typeof getWorker;
      };
    }
  ).MonacoEnvironment = { getWorker };

  loader.config({ monaco });

  loader
    .init()
    .then((monacoInstance) => {
      monacoInstance.languages.register({ id: 'zenith' });
      monacoInstance.languages.setMonarchTokensProvider('zenith', zenithLanguage);
      monacoInstance.languages.setLanguageConfiguration('zenith', zenithConfig);

      const themes = {
        zenith: { base: 'vs', back: '#ffffff00' },
        nord: { base: 'vs-dark', back: '#3b425200' },
        neon: { base: 'vs-dark', back: '#0f081700' },
        dracula: { base: 'vs-dark', back: '#282a3600' },
        neuro: { base: 'vs', back: '#fdfdfd00' },
      };

      Object.entries(themes).forEach(([name, config]) => {
        monacoInstance.editor.defineTheme(`xcode-${name}`, {
          base: config.base as 'vs' | 'vs-dark',
          inherit: true,
          rules: [
            {
              token: 'keyword',
              foreground:
                name === 'zenith' ? '0071E3' : name === 'neuro' ? '2E8B57' : 'D783FF',
              fontStyle: 'bold',
            },
            {
              token: 'keyword.type',
              foreground: name === 'zenith' ? '0B84FF' : '5EB9FF',
            },
            {
              token: 'comment',
              foreground: name === 'zenith' ? '8A8A8F' : '6C7986',
              fontStyle: 'italic',
            },
            {
              token: 'string',
              foreground: name === 'zenith' ? 'C2513F' : 'FC6A5D',
            },
            {
              token: 'number',
              foreground: name === 'zenith' ? '8A6F1F' : 'D0BF69',
            },
            {
              token: 'type.identifier',
              foreground: name === 'zenith' ? '0F4C81' : 'A9E7FF',
            },
          ],
          colors: {
            'editor.background': config.back,
            'editor.foreground':
              name === 'zenith' ? '#1D1D1F' : name === 'neuro' ? '#333333' : '#FFFFFF',
            'editorLineNumber.foreground': name === 'zenith' ? '#B0B0B5' : '#5A5A5A',
            'editorLineNumber.activeForeground': name === 'zenith' ? '#6D6D73' : '#B0B0B0',
            'editor.lineHighlightBackground':
              name === 'zenith' ? '#0071E30A' : name === 'neuro' ? '#eeeeee' : '#2c2c2c',
            'editor.selectionBackground':
              name === 'zenith' ? '#0071E326' : name === 'neuro' ? '#B8D7C4' : '#5A3A80',
            'editor.inactiveSelectionBackground':
              name === 'zenith' ? '#0071E318' : name === 'neuro' ? '#D7E6DD' : '#4C355F',
            'editorCursor.foreground':
              name === 'zenith' ? '#0071E3' : name === 'neuro' ? '#2E8B57' : '#FFFFFF',
            'editorIndentGuide.background1': name === 'zenith' ? '#00000010' : '#FFFFFF12',
            'editorIndentGuide.activeBackground1': name === 'zenith' ? '#00000018' : '#FFFFFF22',
            'editorWhitespace.foreground': name === 'zenith' ? '#00000012' : '#FFFFFF14',
          },
        });
      });

      monacoConfigured = true;
    })
    .catch((error) => {
      console.error('[Monaco] Failed to initialize Monaco:', error);
      monacoConfigured = false;
    });
}

export function getEditorLanguage(fileName: string) {
  if (fileName.endsWith('.zt')) return 'zenith';
  if (fileName.endsWith('.ts') || fileName.endsWith('.tsx')) return 'typescript';
  if (fileName.endsWith('.js') || fileName.endsWith('.jsx')) return 'javascript';
  if (fileName.endsWith('.json')) return 'json';
  if (fileName.endsWith('.html')) return 'html';
  if (fileName.endsWith('.css')) return 'css';
  if (fileName.endsWith('.md')) return 'markdown';
  return 'plaintext';
}

export function toMonacoModelPath(filePath: string) {
  const isAbsolute = /^(?:[a-zA-Z]:[\\/]|\/)/.test(filePath);

  if (isAbsolute) {
    return monaco.Uri.file(filePath).toString();
  }

  const normalized = filePath.replace(/\\/g, '/');
  const uri = monaco.Uri.from({
    scheme: 'inmemory',
    authority: 'workspace',
    path: normalized.startsWith('/') ? normalized : `/${normalized}`,
  });

  return uri.toString();
}
