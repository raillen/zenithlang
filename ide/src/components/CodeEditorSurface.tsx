import { useEffect, useId, useMemo, useRef, type WheelEvent as ReactWheelEvent } from 'react';
import CodeMirror, { EditorView, type ViewUpdate } from '@uiw/react-codemirror';
import { EditorState, type Extension } from '@codemirror/state';
import {
  HighlightStyle,
  StreamLanguage,
  indentUnit,
  syntaxHighlighting,
  type StreamParser,
} from '@codemirror/language';
import { javascript } from '@codemirror/lang-javascript';
import { json } from '@codemirror/lang-json';
import { markdown } from '@codemirror/lang-markdown';
import { html } from '@codemirror/lang-html';
import { css } from '@codemirror/lang-css';
import { tags } from '@lezer/highlight';
import { uiMarker } from '../utils/debugSelectors';
import { isTauriRuntime } from '../utils/tauri';

type ThemeName = 'zenith' | 'nord' | 'neon' | 'dracula' | 'neuro';
type TokenKind = 'plain' | 'keyword' | 'type' | 'string' | 'number' | 'comment' | 'operator';

interface CodeEditorSurfaceProps {
  path: string;
  language: string;
  value: string;
  onChange: (value: string) => void;
  onCursorChange: (line: number, column: number) => void;
  theme: ThemeName;
  fontFamily: string;
  fontSize: number;
  lineHeight: number;
  wrapLines?: boolean;
}

const zenithKeywords = new Set([
  'fn', 'let', 'mut', 'struct', 'trait', 'impl', 'if', 'else',
  'match', 'case', 'for', 'in', 'while', 'return', 'break',
  'continue', 'import', 'as', 'pub', 'async', 'await', 'check',
]);

const zenithTypeKeywords = new Set([
  'int', 'float', 'string', 'bool', 'void', 'any', 'list', 'map', 'grid',
]);

const luaKeywords = new Set([
  'and', 'break', 'do', 'else', 'elseif', 'end', 'false', 'for',
  'function', 'goto', 'if', 'in', 'local', 'nil', 'not', 'or',
  'repeat', 'return', 'then', 'true', 'until', 'while',
]);

const scriptKeywords = new Set([
  'break', 'case', 'catch', 'class', 'const', 'continue', 'debugger', 'default',
  'delete', 'do', 'else', 'enum', 'export', 'extends', 'false', 'finally', 'for',
  'function', 'if', 'import', 'in', 'instanceof', 'let', 'new', 'null', 'return',
  'super', 'switch', 'this', 'throw', 'true', 'try', 'typeof', 'var', 'void',
  'while', 'with', 'yield', 'async', 'await', 'interface', 'type', 'implements',
  'public', 'private', 'protected', 'readonly', 'namespace', 'module',
]);

const zenithParser: StreamParser<{ inBlockComment: boolean; inString: boolean }> = {
  startState: () => ({ inBlockComment: false, inString: false }),
  copyState: (state) => ({ ...state }),
  token(stream, state) {
    if (state.inString) {
      let escaped = false;
      while (!stream.eol()) {
        const ch = stream.next();
        if (ch === '"' && !escaped) {
          state.inString = false;
          break;
        }
        escaped = ch === '\\' && !escaped;
      }
      return 'string';
    }

    if (state.inBlockComment) {
      while (!stream.eol()) {
        if (stream.match('*/')) {
          state.inBlockComment = false;
          break;
        }
        stream.next();
      }
      return 'comment';
    }

    if (stream.eatSpace()) {
      return null;
    }

    if (stream.match('//')) {
      stream.skipToEnd();
      return 'comment';
    }

    if (stream.match('/*')) {
      state.inBlockComment = true;
      return 'comment';
    }

    if (stream.match('"')) {
      state.inString = true;
      return 'string';
    }

    if (stream.match(/\d*\.\d+([eE][\-+]?\d+)?/)) {
      return 'number';
    }
    if (stream.match(/0[xX][0-9a-fA-F]+/)) {
      return 'number';
    }
    if (stream.match(/\d+/)) {
      return 'number';
    }
    if (stream.match(/[A-Z][\w$]*/)) {
      return 'typeName';
    }
    if (stream.match(/[a-z_$][\w$]*/)) {
      const word = stream.current();
      if (zenithTypeKeywords.has(word)) {
        return 'typeName';
      }
      if (zenithKeywords.has(word)) {
        return 'keyword';
      }
      return 'variableName';
    }
    if (stream.match(/[{}()[\]]/)) {
      return 'bracket';
    }
    if (stream.match(/(::|=>|->|==|<=|>=|!=|&&|\|\||\+\+|--|<<=|>>=|>>>|>>|<<|\+=|-=|\*=|\/=|%=|\.\.|[=><!~?:&|+\-*\/^%]+)/)) {
      return 'operator';
    }

    stream.next();
    return 'punctuation';
  },
};

const luaParser: StreamParser<{ inBlockComment: boolean; stringQuote: '"' | "'" | null }> = {
  startState: () => ({ inBlockComment: false, stringQuote: null }),
  copyState: (state) => ({ ...state }),
  token(stream, state) {
    if (state.stringQuote) {
      let escaped = false;
      while (!stream.eol()) {
        const ch = stream.next();
        if (ch === state.stringQuote && !escaped) {
          state.stringQuote = null;
          break;
        }
        escaped = ch === '\\' && !escaped;
      }
      return 'string';
    }

    if (state.inBlockComment) {
      while (!stream.eol()) {
        if (stream.match(']]')) {
          state.inBlockComment = false;
          break;
        }
        stream.next();
      }
      return 'comment';
    }

    if (stream.eatSpace()) {
      return null;
    }

    if (stream.match('--[[')) {
      state.inBlockComment = true;
      return 'comment';
    }
    if (stream.match('--')) {
      stream.skipToEnd();
      return 'comment';
    }
    if (stream.match('"')) {
      state.stringQuote = '"';
      return 'string';
    }
    if (stream.match("'")) {
      state.stringQuote = "'";
      return 'string';
    }
    if (stream.match(/\d*\.\d+([eE][\-+]?\d+)?/)) {
      return 'number';
    }
    if (stream.match(/0[xX][0-9a-fA-F]+/)) {
      return 'number';
    }
    if (stream.match(/\d+/)) {
      return 'number';
    }
    if (stream.match(/[A-Za-z_][\w]*/)) {
      return luaKeywords.has(stream.current()) ? 'keyword' : 'variableName';
    }
    if (stream.match(/[{}()[\]]/)) {
      return 'bracket';
    }
    if (stream.match(/(\.\.\.?|==|~=|<=|>=|[=+\-*\/%^#<>])/)) {
      return 'operator';
    }

    stream.next();
    return 'punctuation';
  },
};

const zenithLanguage = StreamLanguage.define(zenithParser);
const luaLanguage = StreamLanguage.define(luaParser);

function getThemePalette(theme: ThemeName) {
  if (theme === 'nord') {
    return {
      isDark: true,
      background: '#3b4252',
      foreground: '#eceff4',
      gutter: 'rgba(236, 239, 244, 0.58)',
      gutterActive: '#eceff4',
      selection: 'rgba(136, 192, 208, 0.24)',
      line: 'rgba(136, 192, 208, 0.10)',
      cursor: '#88c0d0',
      keyword: '#88c0d0',
      type: '#8fbcbb',
      string: '#a3be8c',
      number: '#ebcb8b',
      comment: '#7d8899',
      operator: '#81a1c1',
    };
  }

  if (theme === 'neon') {
    return {
      isDark: true,
      background: '#120322',
      foreground: '#f5fbff',
      gutter: 'rgba(245, 251, 255, 0.50)',
      gutterActive: '#f5fbff',
      selection: 'rgba(0, 215, 255, 0.22)',
      line: 'rgba(0, 215, 255, 0.08)',
      cursor: '#00d7ff',
      keyword: '#00d7ff',
      type: '#7cf7ff',
      string: '#ff9f6e',
      number: '#ffe27a',
      comment: '#8f86a6',
      operator: '#64d5ff',
    };
  }

  if (theme === 'dracula') {
    return {
      isDark: true,
      background: '#282a36',
      foreground: '#f8f8f2',
      gutter: 'rgba(248, 248, 242, 0.46)',
      gutterActive: '#f8f8f2',
      selection: 'rgba(189, 147, 249, 0.22)',
      line: 'rgba(189, 147, 249, 0.08)',
      cursor: '#bd93f9',
      keyword: '#ff79c6',
      type: '#8be9fd',
      string: '#f1fa8c',
      number: '#ffb86c',
      comment: '#6272a4',
      operator: '#bd93f9',
    };
  }

  if (theme === 'neuro') {
    return {
      isDark: false,
      background: '#fffdf7',
      foreground: '#2f2d28',
      gutter: 'rgba(47, 45, 40, 0.42)',
      gutterActive: '#2f2d28',
      selection: 'rgba(46, 139, 87, 0.16)',
      line: 'rgba(46, 139, 87, 0.07)',
      cursor: '#2e8b57',
      keyword: '#2e8b57',
      type: '#2d6d58',
      string: '#b45b3c',
      number: '#9f6a16',
      comment: '#8b857a',
      operator: '#3f6a5f',
    };
  }

  return {
    isDark: false,
    background: '#f8f9fb',
    foreground: '#1d1d1f',
    gutter: 'rgba(29, 29, 31, 0.40)',
    gutterActive: '#1d1d1f',
    selection: 'rgba(0, 113, 227, 0.16)',
    line: 'rgba(0, 113, 227, 0.07)',
    cursor: '#0071e3',
    keyword: '#0071e3',
    type: '#0f4c81',
    string: '#c2513f',
    number: '#8a6f1f',
    comment: '#8a8a8f',
    operator: '#3d6f9d',
  };
}

function getLanguageExtension(language: string): Extension[] {
  switch (language) {
    case 'zenith':
      return [zenithLanguage];
    case 'lua':
      return [luaLanguage];
    case 'typescript':
      return [javascript({ typescript: true })];
    case 'javascript':
      return [javascript()];
    case 'json':
      return [json()];
    case 'html':
      return [html()];
    case 'css':
      return [css()];
    case 'markdown':
      return [markdown()];
    default:
      return [];
  }
}

function escapeHtml(value: string) {
  return value
    .replace(/&/g, '&amp;')
    .replace(/</g, '&lt;')
    .replace(/>/g, '&gt;')
    .replace(/"/g, '&quot;');
}

function renderToken(kind: TokenKind, value: string) {
  const escaped = escapeHtml(value);
  return kind === 'plain' ? escaped : `<span class="safe-editor-token-${kind}">${escaped}</span>`;
}

function highlightZenith(value: string) {
  let result = '';
  let index = 0;
  let inBlockComment = false;

  while (index < value.length) {
    if (inBlockComment) {
      const closeIndex = value.indexOf('*/', index);
      if (closeIndex === -1) {
        result += renderToken('comment', value.slice(index));
        break;
      }
      result += renderToken('comment', value.slice(index, closeIndex + 2));
      index = closeIndex + 2;
      inBlockComment = false;
      continue;
    }

    const remaining = value.slice(index);

    if (remaining.startsWith('//')) {
      const lineEnd = value.indexOf('\n', index);
      const end = lineEnd === -1 ? value.length : lineEnd;
      result += renderToken('comment', value.slice(index, end));
      index = end;
      continue;
    }

    if (remaining.startsWith('/*')) {
      inBlockComment = true;
      continue;
    }

    const char = value[index];

    if (char === '"') {
      let end = index + 1;
      let escaped = false;
      while (end < value.length) {
        const current = value[end];
        if (current === '"' && !escaped) {
          end += 1;
          break;
        }
        escaped = current === '\\' && !escaped;
        if (current !== '\\') {
          escaped = false;
        }
        end += 1;
      }
      result += renderToken('string', value.slice(index, end));
      index = end;
      continue;
    }

    const numberMatch = remaining.match(/^(?:0[xX][0-9a-fA-F]+|\d*\.\d+(?:[eE][\-+]?\d+)?|\d+)/);
    if (numberMatch) {
      result += renderToken('number', numberMatch[0]);
      index += numberMatch[0].length;
      continue;
    }

    const identifierMatch = remaining.match(/^[A-Za-z_$][\w$]*/);
    if (identifierMatch) {
      const identifier = identifierMatch[0];
      if (zenithTypeKeywords.has(identifier) || /^[A-Z]/.test(identifier)) {
        result += renderToken('type', identifier);
      } else if (zenithKeywords.has(identifier)) {
        result += renderToken('keyword', identifier);
      } else {
        result += renderToken('plain', identifier);
      }
      index += identifier.length;
      continue;
    }

    const operatorMatch = remaining.match(/^(?:::|=>|->|==|<=|>=|!=|&&|\|\||\+\+|--|<<=|>>=|>>>|>>|<<|\+=|-=|\*=|\/=|%=|\.\.|[=><!~?:&|+\-*\/^%]+)/);
    if (operatorMatch) {
      result += renderToken('operator', operatorMatch[0]);
      index += operatorMatch[0].length;
      continue;
    }

    result += renderToken('plain', char);
    index += 1;
  }

  return result;
}

function highlightLua(value: string) {
  let result = '';
  let index = 0;
  let inBlockComment = false;

  while (index < value.length) {
    if (inBlockComment) {
      const closeIndex = value.indexOf(']]', index);
      if (closeIndex === -1) {
        result += renderToken('comment', value.slice(index));
        break;
      }
      result += renderToken('comment', value.slice(index, closeIndex + 2));
      index = closeIndex + 2;
      inBlockComment = false;
      continue;
    }

    const remaining = value.slice(index);

    if (remaining.startsWith('--[[')) {
      inBlockComment = true;
      continue;
    }

    if (remaining.startsWith('--')) {
      const lineEnd = value.indexOf('\n', index);
      const end = lineEnd === -1 ? value.length : lineEnd;
      result += renderToken('comment', value.slice(index, end));
      index = end;
      continue;
    }

    const char = value[index];
    if (char === '"' || char === "'") {
      const quote = char;
      let end = index + 1;
      let escaped = false;
      while (end < value.length) {
        const current = value[end];
        if (current === quote && !escaped) {
          end += 1;
          break;
        }
        escaped = current === '\\' && !escaped;
        if (current !== '\\') {
          escaped = false;
        }
        end += 1;
      }
      result += renderToken('string', value.slice(index, end));
      index = end;
      continue;
    }

    const numberMatch = remaining.match(/^(?:0[xX][0-9a-fA-F]+|\d*\.\d+(?:[eE][\-+]?\d+)?|\d+)/);
    if (numberMatch) {
      result += renderToken('number', numberMatch[0]);
      index += numberMatch[0].length;
      continue;
    }

    const identifierMatch = remaining.match(/^[A-Za-z_][\w]*/);
    if (identifierMatch) {
      const identifier = identifierMatch[0];
      result += renderToken(luaKeywords.has(identifier) ? 'keyword' : 'plain', identifier);
      index += identifier.length;
      continue;
    }

    const operatorMatch = remaining.match(/^(?:\.\.\.?|==|~=|<=|>=|[=+\-*\/%^#<>])/);
    if (operatorMatch) {
      result += renderToken('operator', operatorMatch[0]);
      index += operatorMatch[0].length;
      continue;
    }

    result += renderToken('plain', char);
    index += 1;
  }

  return result;
}

function highlightScript(value: string) {
  let result = '';
  let index = 0;

  while (index < value.length) {
    const remaining = value.slice(index);

    if (remaining.startsWith('//')) {
      const lineEnd = value.indexOf('\n', index);
      const end = lineEnd === -1 ? value.length : lineEnd;
      result += renderToken('comment', value.slice(index, end));
      index = end;
      continue;
    }

    if (remaining.startsWith('/*')) {
      const closeIndex = value.indexOf('*/', index + 2);
      const end = closeIndex === -1 ? value.length : closeIndex + 2;
      result += renderToken('comment', value.slice(index, end));
      index = end;
      continue;
    }

    const char = value[index];
    if (char === '"' || char === "'" || char === '`') {
      const quote = char;
      let end = index + 1;
      let escaped = false;
      while (end < value.length) {
        const current = value[end];
        if (current === quote && !escaped) {
          end += 1;
          break;
        }
        escaped = current === '\\' && !escaped;
        if (current !== '\\') {
          escaped = false;
        }
        end += 1;
      }
      result += renderToken('string', value.slice(index, end));
      index = end;
      continue;
    }

    const numberMatch = remaining.match(/^(?:0[xX][0-9a-fA-F]+|\d*\.\d+(?:[eE][\-+]?\d+)?|\d+)/);
    if (numberMatch) {
      result += renderToken('number', numberMatch[0]);
      index += numberMatch[0].length;
      continue;
    }

    const identifierMatch = remaining.match(/^[A-Za-z_$][\w$]*/);
    if (identifierMatch) {
      const identifier = identifierMatch[0];
      result += renderToken(scriptKeywords.has(identifier) ? 'keyword' : 'plain', identifier);
      index += identifier.length;
      continue;
    }

    const operatorMatch = remaining.match(/^(?:===|!==|=>|==|<=|>=|!=|&&|\|\||\?\?|\+\+|--|<<=|>>=|>>>|>>|<<|\+=|-=|\*=|\/=|%=|[=><!~?:&|+\-*\/^%]+)/);
    if (operatorMatch) {
      result += renderToken('operator', operatorMatch[0]);
      index += operatorMatch[0].length;
      continue;
    }

    result += renderToken('plain', char);
    index += 1;
  }

  return result;
}

function highlightDocument(value: string, language: string) {
  switch (language) {
    case 'zenith':
      return highlightZenith(value);
    case 'lua':
      return highlightLua(value);
    case 'javascript':
    case 'typescript':
    case 'json':
    case 'css':
    case 'html':
      return highlightScript(value);
    default:
      return escapeHtml(value);
  }
}

function SafeTextEditor({
  path,
  language,
  value,
  onChange,
  onCursorChange,
  palette,
  fontFamily,
  fontSize,
  lineHeight,
  wrapLines,
}: CodeEditorSurfaceProps & { palette: ReturnType<typeof getThemePalette> }) {
  const styleId = useId().replace(/:/g, '-');
  const textareaRef = useRef<HTMLTextAreaElement | null>(null);
  const mirrorScrollRef = useRef<HTMLDivElement | null>(null);
  const gutterScrollRef = useRef<HTMLDivElement | null>(null);
  const isDesktopSafeEditor = useMemo(() => isTauriRuntime(), []);

  const highlightedHtml = useMemo(() => highlightDocument(value, language), [language, value]);
  const lineCount = useMemo(() => Math.max(1, value.split('\n').length), [value]);
  const gutterWidth = useMemo(() => Math.max(48, String(lineCount).length * 12 + 22), [lineCount]);
  const lineNumbers = useMemo(
    () => Array.from({ length: lineCount }, (_, index) => index + 1),
    [lineCount]
  );

  const syncCursor = () => {
    const textarea = textareaRef.current;
    if (!textarea) {
      return;
    }

    const head = textarea.selectionStart;
    const before = value.slice(0, head);
    const lastBreak = before.lastIndexOf('\n');
    const line = before.split('\n').length;
    const column = head - lastBreak;
    onCursorChange(line, column);
  };

  useEffect(() => {
    syncCursor();
  }, [value]);

  const handleScroll = () => {
    const textarea = textareaRef.current;
    if (!textarea) {
      return;
    }

    if (mirrorScrollRef.current) {
      mirrorScrollRef.current.scrollTop = textarea.scrollTop;
      mirrorScrollRef.current.scrollLeft = textarea.scrollLeft;
    }

    if (gutterScrollRef.current) {
      gutterScrollRef.current.scrollTop = textarea.scrollTop;
    }
  };

  const handleWheel = (event: ReactWheelEvent<HTMLDivElement>) => {
    const textarea = textareaRef.current;
    if (!textarea) {
      return;
    }

    const nextTop = textarea.scrollTop + event.deltaY;
    const nextLeft =
      textarea.scrollLeft +
      (Math.abs(event.deltaX) > 0 ? event.deltaX : event.shiftKey ? event.deltaY : 0);

    textarea.scrollTop = nextTop;
    textarea.scrollLeft = nextLeft;
    handleScroll();
    event.preventDefault();
  };

  const handleChange = (nextValue: string) => {
    onChange(nextValue);
    window.requestAnimationFrame(syncCursor);
  };

  const handleTabKey = (event: React.KeyboardEvent<HTMLTextAreaElement>) => {
    if (event.key !== 'Tab') {
      return;
    }

    event.preventDefault();
    const textarea = event.currentTarget;
    const start = textarea.selectionStart;
    const end = textarea.selectionEnd;
    const indentation = '  ';
    const nextValue = `${value.slice(0, start)}${indentation}${value.slice(end)}`;
    onChange(nextValue);

    window.requestAnimationFrame(() => {
      const target = textareaRef.current;
      if (!target) {
        return;
      }
      const cursor = start + indentation.length;
      target.selectionStart = cursor;
      target.selectionEnd = cursor;
      syncCursor();
      handleScroll();
    });
  };

  if (!isDesktopSafeEditor) {
    return null;
  }

  return (
    <div
      className={`safe-editor-${styleId} flex h-full min-h-0 overflow-hidden`}
      style={{ backgroundColor: palette.background, color: palette.foreground }}
      onWheelCapture={handleWheel}
      {...uiMarker('code-editor-surface-safe', { filePath: path, language })}
    >
      <style>{`
        .safe-editor-${styleId} textarea::selection {
          background: ${palette.selection};
        }
        .safe-editor-${styleId} .safe-editor-token-keyword {
          color: ${palette.keyword};
          font-weight: 600;
        }
        .safe-editor-${styleId} .safe-editor-token-type {
          color: ${palette.type};
        }
        .safe-editor-${styleId} .safe-editor-token-string {
          color: ${palette.string};
        }
        .safe-editor-${styleId} .safe-editor-token-number {
          color: ${palette.number};
        }
        .safe-editor-${styleId} .safe-editor-token-comment {
          color: ${palette.comment};
          font-style: italic;
        }
        .safe-editor-${styleId} .safe-editor-token-operator {
          color: ${palette.operator};
        }
      `}</style>

      <div
        className="shrink-0 border-r"
        style={{
          width: `${gutterWidth}px`,
          borderColor: palette.line,
          backgroundColor: palette.background,
          color: palette.gutter,
        }}
        {...uiMarker('code-editor-safe-gutter', { filePath: path })}
      >
        <div ref={gutterScrollRef} className="h-full overflow-hidden">
          <div
            style={{
              padding: '10px 8px 24px 6px',
              fontFamily,
              fontSize: `${fontSize}px`,
              lineHeight: `${lineHeight}px`,
            }}
          >
            {lineNumbers.map((lineNumber) => (
              <div
                key={lineNumber}
                className="select-none text-right"
                style={{ minHeight: `${lineHeight}px` }}
              >
                {lineNumber}
              </div>
            ))}
          </div>
        </div>
      </div>

      <div className="relative min-w-0 flex-1 overflow-hidden" {...uiMarker('code-editor-safe-scroll-host', { filePath: path })}>
        <div ref={mirrorScrollRef} aria-hidden className="absolute inset-0 overflow-hidden">
          <pre
            className="m-0 min-h-full"
            style={{
              padding: '10px 16px 24px 12px',
              fontFamily,
              fontSize: `${fontSize}px`,
              lineHeight: `${lineHeight}px`,
              whiteSpace: wrapLines ? 'pre-wrap' : 'pre',
              overflowWrap: wrapLines ? 'anywhere' : 'normal',
            }}
          >
            <code dangerouslySetInnerHTML={{ __html: highlightedHtml.length > 0 ? highlightedHtml : ' ' }} />
          </pre>
        </div>

        <textarea
          ref={textareaRef}
          value={value}
          onChange={(event) => handleChange(event.target.value)}
          onScroll={handleScroll}
          onClick={syncCursor}
          onKeyUp={syncCursor}
          onSelect={syncCursor}
          onKeyDown={handleTabKey}
          spellCheck={false}
          wrap={wrapLines ? 'soft' : 'off'}
          className="absolute inset-0 h-full w-full resize-none border-0 bg-transparent outline-none"
          style={{
            padding: '10px 16px 24px 12px',
            fontFamily,
            fontSize: `${fontSize}px`,
            lineHeight: `${lineHeight}px`,
            whiteSpace: wrapLines ? 'pre-wrap' : 'pre',
            overflowWrap: wrapLines ? 'anywhere' : 'normal',
            color: 'transparent',
            WebkitTextFillColor: 'transparent',
            caretColor: palette.cursor,
            tabSize: 2,
          }}
          {...uiMarker('code-editor-safe-textarea', { filePath: path, language })}
        />
      </div>
    </div>
  );
}

export function CodeEditorSurface({
  path,
  language,
  value,
  onChange,
  onCursorChange,
  theme,
  fontFamily,
  fontSize,
  lineHeight,
  wrapLines = false,
}: CodeEditorSurfaceProps) {
  const palette = useMemo(() => getThemePalette(theme), [theme]);
  const isDesktopSafeEditor = false;
  const codeMirrorHostRef = useRef<HTMLDivElement | null>(null);

  const handleCodeMirrorWheel = (event: ReactWheelEvent<HTMLDivElement>) => {
    const scroller = codeMirrorHostRef.current?.querySelector('.cm-scroller') as HTMLElement | null;
    if (!scroller) {
      return;
    }

    const canScrollVertically = scroller.scrollHeight > scroller.clientHeight;
    const canScrollHorizontally = scroller.scrollWidth > scroller.clientWidth;

    if (!canScrollVertically && !canScrollHorizontally) {
      return;
    }

    if (canScrollVertically && Math.abs(event.deltaY) > 0) {
      scroller.scrollTop += event.deltaY;
    }

    const horizontalDelta =
      Math.abs(event.deltaX) > 0 ? event.deltaX : event.shiftKey ? event.deltaY : 0;

    if (canScrollHorizontally && Math.abs(horizontalDelta) > 0) {
      scroller.scrollLeft += horizontalDelta;
    }

    event.preventDefault();
  };

  const extensions = useMemo(() => {
    const editorTheme = EditorView.theme(
      {
        '&': {
          height: '100%',
          minHeight: '0',
          color: palette.foreground,
          backgroundColor: palette.background,
          fontFamily,
          fontSize: `${fontSize}px`,
        },
        '.cm-editor': {
          position: 'relative',
          zIndex: '1',
          height: '100%',
          minHeight: '0',
        },
        '.cm-scroller': {
          height: '100%',
          minHeight: '0',
          fontFamily,
          lineHeight: `${lineHeight}px`,
          overflow: 'auto',
        },
        '.cm-content': {
          minHeight: '100%',
          padding: '10px 0 24px',
          caretColor: palette.cursor,
        },
        '.cm-line': {
          padding: '0 16px 0 12px',
        },
        '.cm-focused': {
          outline: 'none',
        },
        '.cm-gutters': {
          height: '100%',
          backgroundColor: palette.background,
          color: palette.gutter,
          border: 'none',
          minWidth: '56px',
        },
        '.cm-activeLine': {
          backgroundColor: palette.line,
        },
        '.cm-activeLineGutter': {
          color: palette.gutterActive,
          backgroundColor: palette.line,
        },
        '.cm-selectionBackground, ::selection': {
          backgroundColor: `${palette.selection} !important`,
        },
        '.cm-cursor, .cm-dropCursor': {
          borderLeftColor: palette.cursor,
        },
        '.cm-panels': {
          backgroundColor: palette.background,
          color: palette.foreground,
        },
      },
      { dark: palette.isDark }
    );

    const highlightTheme = syntaxHighlighting(
      HighlightStyle.define([
        { tag: tags.keyword, color: palette.keyword, fontWeight: '600' },
        { tag: [tags.typeName, tags.className], color: palette.type },
        { tag: [tags.string, tags.special(tags.string)], color: palette.string },
        { tag: [tags.number, tags.bool, tags.null], color: palette.number },
        { tag: tags.comment, color: palette.comment, fontStyle: 'italic' },
        { tag: [tags.operator, tags.punctuation], color: palette.operator },
        { tag: [tags.variableName, tags.propertyName], color: palette.foreground },
      ])
    );

    const updateCursor = EditorView.updateListener.of((update: ViewUpdate) => {
      if (
        !update.docChanged &&
        !update.selectionSet &&
        update.viewportChanged === false &&
        update.focusChanged === false
      ) {
        return;
      }

      const head = update.state.selection.main.head;
      const line = update.state.doc.lineAt(head);
      onCursorChange(line.number, head - line.from + 1);
    });

    return [
      ...getLanguageExtension(language),
      editorTheme,
      highlightTheme,
      updateCursor,
      EditorState.tabSize.of(2),
      indentUnit.of('  '),
      ...(wrapLines ? [EditorView.lineWrapping] : []),
    ];
  }, [fontFamily, fontSize, language, lineHeight, onCursorChange, palette, wrapLines]);

  if (isDesktopSafeEditor) {
    return (
      <SafeTextEditor
        path={path}
        language={language}
        value={value}
        onChange={onChange}
        onCursorChange={onCursorChange}
        theme={theme}
        fontFamily={fontFamily}
        fontSize={fontSize}
        lineHeight={lineHeight}
        wrapLines={wrapLines}
        palette={palette}
      />
    );
  }

  return (
    <div
      ref={codeMirrorHostRef}
      className="relative z-20 flex h-full min-h-0 flex-col overflow-hidden"
      onWheelCapture={handleCodeMirrorWheel}
      {...uiMarker('code-editor-surface', { filePath: path, language })}
    >
      <CodeMirror
        className="h-full min-h-0"
        value={value}
        height="100%"
        theme="none"
        basicSetup={{
          foldGutter: false,
          highlightActiveLine: true,
          highlightActiveLineGutter: true,
        }}
        extensions={extensions}
        onCreateEditor={(view) => {
          const head = view.state.selection.main.head;
          const line = view.state.doc.lineAt(head);
          onCursorChange(line.number, head - line.from + 1);
        }}
        onChange={onChange}
        indentWithTab
      />
    </div>
  );
}
