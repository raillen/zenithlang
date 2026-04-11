import { StreamLanguage } from '@codemirror/language';

const KEYWORDS = new Set([
  'func',
  'var',
  'if',
  'else',
  'elif',
  'while',
  'for',
  'in',
  'return',
  'break',
  'continue',
  'import',
  'export',
  'type',
  'interface',
  'outcome',
  'optional',
  'try',
  'panic',
  'test',
  'module',
  'struct',
  'enum',
  'pub',
  'repeat',
  'match',
  'case',
  'do',
  'end',
  'namespace',
  'as',
]);

const ATOMS = new Set(['true', 'false', 'none', 'null']);

const zenithStreamParser = {
  startState() {
    return { inBlockComment: false };
  },

  token(stream, state) {
    if (state.inBlockComment) {
      let currentChar = stream.next();

      while (currentChar != null) {
        if (currentChar === '*' && stream.eat('/')) {
          state.inBlockComment = false;
          break;
        }

        currentChar = stream.next();
      }

      return 'comment';
    }

    if (stream.eatSpace()) {
      return null;
    }

    if (stream.match('--')) {
      stream.skipToEnd();
      return 'comment';
    }

    if (stream.match('//')) {
      stream.skipToEnd();
      return 'comment';
    }

    if (stream.match('/*')) {
      state.inBlockComment = true;
      return 'comment';
    }

    if (stream.match(/^"(?:[^"\\]|\\.)*"?/)) {
      return 'string';
    }

    if (stream.match(/^'(?:[^'\\]|\\.)*'?/)) {
      return 'string';
    }

    if (stream.match(/^-?\d+(?:\.\d+)?(?:[eE][+-]?\d+)?/)) {
      return 'number';
    }

    if (stream.match(/^@[A-Za-z_]\w*/)) {
      return 'property';
    }

    if (stream.match(/^[A-Za-z_]\w*/)) {
      const token = stream.current();

      if (KEYWORDS.has(token)) {
        return 'keyword';
      }

      if (ATOMS.has(token)) {
        return 'atom';
      }

      if (stream.match(/^\s*\(/, false)) {
        return 'def';
      }

      return 'variableName';
    }

    if (stream.match(/^(?:==|!=|<=|>=|\+=|-=|\*=|\/=|->|=>|\.\.|[+\-*/%^=<>])/)) {
      return 'operator';
    }

    if (stream.match(/^[()[\]{},.:]/)) {
      return 'punctuation';
    }

    stream.next();
    return null;
  },

  languageData: {
    commentTokens: {
      line: '--',
      block: { open: '/*', close: '*/' },
    },
  },
};

export const zenithLanguage = StreamLanguage.define(zenithStreamParser);
