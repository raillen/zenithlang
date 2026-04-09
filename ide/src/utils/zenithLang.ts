import { ZENITH_BUILTINS } from "./zenithDocs";

export function registerZenithLanguage(monaco: any) {
  // 1. Syntax Tokens (Monarch)
  monaco.languages.register({ id: 'zenith' });
  monaco.languages.setMonarchTokensProvider('zenith', {
    tokenizer: {
      root: [
        [/\b(func|fn|let|if|else|while|return|match|check|grid|outcome|optional|import|as|from|type|interface|class|public|private|module|async)\b/, 'keyword'],
        [/\b(true|false|null)\b/, 'constant'],
        [/"([^"\\]|\\.)*"/, 'string'],
        [/\/\/.*$/, 'comment'],
        [/[0-9]+/, 'number'],
        [/[{}()\[\]]/, '@brackets'],
        [/[A-Z][a-zA-Z0-9]*/, 'type.identifier'],
      ]
    }
  });

  // 2. Hover Provider (Documentation)
  monaco.languages.registerHoverProvider('zenith', {
    provideHover: (model: any, position: any) => {
      const word = model.getWordAtPosition(position);
      if (!word) return null;

      // Check current word + possible module prefix
      const line = model.getLineContent(position.lineNumber);
      const fullLine = line.substring(0, word.endColumn - 1);
      const match = fullLine.match(/([\w.]+)\.?$/);
      const fullWord = match ? match[1] : word.word;

      const doc = ZENITH_BUILTINS[fullWord];

      if (doc) {
        return {
          range: new monaco.Range(position.lineNumber, word.startColumn, position.lineNumber, word.endColumn),
          contents: [
            { value: `**${doc.signature}**` },
            { value: doc.description },
            { value: '```zenith\n' + doc.example + '\n```' }
          ]
        };
      }
      return null;
    }
  });

  // 3. Completion Item Provider (Snippets)
  monaco.languages.registerCompletionItemProvider('zenith', {
    provideCompletionItems: (model: any, position: any) => {
      const suggestions = Object.entries(ZENITH_BUILTINS).map(([key, doc]) => ({
        label: key,
        kind: monaco.languages.CompletionItemKind.Function,
        insertText: key,
        detail: doc.signature,
        documentation: doc.description,
        range: null // Automatic
      }));

      // Add snippets
      suggestions.push({
        label: 'func',
        kind: monaco.languages.CompletionItemKind.Snippet,
        insertText: 'func ${1:name}(${2:args}) {\n\t$0\n}',
        insertTextRules: monaco.languages.CompletionItemInsertTextRule.InsertAsSnippet,
        detail: 'Function Definition',
        documentation: 'Create a new Zenith functions',
        range: null
      } as any);

      return { suggestions };
    }
  });
}
