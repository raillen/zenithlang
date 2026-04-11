import { useWorkspaceStore } from "../store/useWorkspaceStore";
import { getRuntimeProfileForSettingsKey, RuntimeProfileId } from "./runtimeProfiles";
import { ZENITH_BUILTINS } from "./zenithDocs";
import {
  applyWorkspaceEdit,
  getFileContextFromModel,
  requestLspCompletions,
  requestLspDefinition,
  requestLspHover,
  requestLspReferences,
  requestLspRename,
} from "./lspClient";

type MonacoLike = any;

interface SemanticRuntimeConfig {
  runtimeId: RuntimeProfileId;
  languageId: string;
  keywords: string[];
  snippets: SemanticSnippet[];
  declarationPatterns: SemanticDeclarationPattern[];
}

interface SemanticSnippet {
  label: string;
  detail: string;
  insertText: string;
  documentation: string;
}

interface SemanticDeclarationPattern {
  kind: "function" | "class" | "variable" | "constant" | "module";
  pattern: RegExp;
}

const registeredLanguageIds = new Set<string>();
const registeredProviderIds = new Set<string>();
const registeredHoverProviderIds = new Set<string>();
const registeredDefinitionProviderIds = new Set<string>();
const registeredReferenceProviderIds = new Set<string>();
const registeredRenameProviderIds = new Set<string>();

const SEMANTIC_RUNTIME_CONFIGS: SemanticRuntimeConfig[] = [
  {
    runtimeId: "zenith",
    languageId: "zenith",
    keywords: [
      "func",
      "struct",
      "interface",
      "const",
      "var",
      "let",
      "if",
      "else",
      "while",
      "return",
      "match",
      "check",
      "import",
      "module",
      "true",
      "false",
      "null",
    ],
    snippets: [
      {
        label: "func",
        detail: "Zenith function",
        insertText: "func ${1:name}(${2:args}) {\n\t$0\n}",
        documentation: "Create a Zenith function.",
      },
      {
        label: "struct",
        detail: "Zenith struct",
        insertText: "struct ${1:Name} {\n\t${2:field}: ${3:Type}\n}",
        documentation: "Create a Zenith struct.",
      },
      {
        label: "match",
        detail: "Zenith match expression",
        insertText: "match ${1:value} {\n\t${2:case} => ${3:result},\n\t_ => $0\n}",
        documentation: "Create a Zenith match expression.",
      },
    ],
    declarationPatterns: [
      { kind: "function", pattern: /\bfunc\s+([A-Za-z_][A-Za-z0-9_]*)\s*\(/g },
      { kind: "class", pattern: /\b(?:struct|interface|type)\s+([A-Za-z_][A-Za-z0-9_]*)\b/g },
      { kind: "constant", pattern: /\bconst\s+([A-Za-z_][A-Za-z0-9_]*)\b/g },
      { kind: "variable", pattern: /\b(?:let|var)\s+([A-Za-z_][A-Za-z0-9_]*)\b/g },
      { kind: "module", pattern: /\bimport\s+([A-Za-z_][A-Za-z0-9_.]*)\b/g },
    ],
  },
  {
    runtimeId: "node",
    languageId: "typescript",
    keywords: ["async", "await", "const", "let", "function", "interface", "type", "class", "import", "export", "return"],
    snippets: [
      {
        label: "async function",
        detail: "TypeScript async function",
        insertText: "async function ${1:name}(${2:args}) {\n\t$0\n}",
        documentation: "Create an async TypeScript function.",
      },
    ],
    declarationPatterns: [
      { kind: "function", pattern: /\b(?:async\s+)?function\s+([A-Za-z_$][A-Za-z0-9_$]*)\s*\(/g },
      { kind: "function", pattern: /\b(?:const|let|var)\s+([A-Za-z_$][A-Za-z0-9_$]*)\s*=\s*(?:async\s*)?\(/g },
      { kind: "class", pattern: /\b(?:class|interface|type|enum)\s+([A-Za-z_$][A-Za-z0-9_$]*)\b/g },
      { kind: "variable", pattern: /\b(?:const|let|var)\s+([A-Za-z_$][A-Za-z0-9_$]*)\b/g },
      { kind: "module", pattern: /\bimport\s+(?:type\s+)?(?:\{?\s*)?([A-Za-z_$][A-Za-z0-9_$]*)/g },
    ],
  },
  {
    runtimeId: "node",
    languageId: "javascript",
    keywords: ["async", "await", "const", "let", "function", "class", "import", "export", "return", "require"],
    snippets: [
      {
        label: "async function",
        detail: "JavaScript async function",
        insertText: "async function ${1:name}(${2:args}) {\n\t$0\n}",
        documentation: "Create an async JavaScript function.",
      },
    ],
    declarationPatterns: [
      { kind: "function", pattern: /\b(?:async\s+)?function\s+([A-Za-z_$][A-Za-z0-9_$]*)\s*\(/g },
      { kind: "function", pattern: /\b(?:const|let|var)\s+([A-Za-z_$][A-Za-z0-9_$]*)\s*=\s*(?:async\s*)?\(/g },
      { kind: "class", pattern: /\bclass\s+([A-Za-z_$][A-Za-z0-9_$]*)\b/g },
      { kind: "variable", pattern: /\b(?:const|let|var)\s+([A-Za-z_$][A-Za-z0-9_$]*)\b/g },
      { kind: "module", pattern: /\b(?:import|require)\s*\(?\s*["']([^"']+)["']/g },
    ],
  },
  {
    runtimeId: "dotnet",
    languageId: "csharp",
    keywords: ["using", "namespace", "class", "record", "struct", "interface", "public", "private", "async", "await", "return", "var", "new"],
    snippets: [
      {
        label: "public class",
        detail: "C# public class",
        insertText: "public class ${1:Name}\n{\n\t$0\n}",
        documentation: "Create a C# class.",
      },
      {
        label: "async Task",
        detail: "C# async Task method",
        insertText: "public async Task ${1:Name}Async(${2})\n{\n\t$0\n}",
        documentation: "Create an async Task method.",
      },
    ],
    declarationPatterns: [
      { kind: "class", pattern: /\b(?:class|record|struct|interface|enum)\s+([A-Za-z_][A-Za-z0-9_]*)\b/g },
      { kind: "function", pattern: /\b(?:public|private|protected|internal|static|async|\s)+\s*[A-Za-z0-9_<>,?]+\s+([A-Za-z_][A-Za-z0-9_]*)\s*\(/g },
      { kind: "variable", pattern: /\b(?:var|string|int|bool|double|decimal|float|long)\s+([A-Za-z_][A-Za-z0-9_]*)\b/g },
      { kind: "module", pattern: /\busing\s+([A-Za-z_][A-Za-z0-9_.]*)/g },
    ],
  },
  {
    runtimeId: "rust",
    languageId: "rust",
    keywords: ["fn", "let", "mut", "struct", "enum", "trait", "impl", "use", "mod", "pub", "async", "await", "match", "Result", "Option"],
    snippets: [
      {
        label: "fn",
        detail: "Rust function",
        insertText: "fn ${1:name}(${2:args}) {\n\t$0\n}",
        documentation: "Create a Rust function.",
      },
      {
        label: "impl",
        detail: "Rust impl block",
        insertText: "impl ${1:Type} {\n\t$0\n}",
        documentation: "Create an impl block.",
      },
    ],
    declarationPatterns: [
      { kind: "function", pattern: /\bfn\s+([A-Za-z_][A-Za-z0-9_]*)\s*\(/g },
      { kind: "class", pattern: /\b(?:struct|enum|trait|type)\s+([A-Za-z_][A-Za-z0-9_]*)\b/g },
      { kind: "variable", pattern: /\blet\s+(?:mut\s+)?([A-Za-z_][A-Za-z0-9_]*)\b/g },
      { kind: "module", pattern: /\b(?:use|mod)\s+([A-Za-z_][A-Za-z0-9_:]*)/g },
    ],
  },
  {
    runtimeId: "lua",
    languageId: "lua",
    keywords: ["function", "local", "return", "require", "if", "then", "elseif", "else", "end", "for", "while", "do"],
    snippets: [
      {
        label: "function",
        detail: "Lua function",
        insertText: "function ${1:name}(${2:args})\n\t$0\nend",
        documentation: "Create a Lua function.",
      },
    ],
    declarationPatterns: [
      { kind: "function", pattern: /\bfunction\s+([A-Za-z_][A-Za-z0-9_.:]*)\s*\(/g },
      { kind: "function", pattern: /\blocal\s+function\s+([A-Za-z_][A-Za-z0-9_]*)\s*\(/g },
      { kind: "variable", pattern: /\blocal\s+([A-Za-z_][A-Za-z0-9_]*)\b/g },
      { kind: "module", pattern: /\brequire\s*\(?\s*["']([^"']+)["']/g },
    ],
  },
  {
    runtimeId: "ruby",
    languageId: "ruby",
    keywords: ["def", "class", "module", "require", "include", "attr_reader", "attr_accessor", "do", "end", "if", "else", "elsif"],
    snippets: [
      {
        label: "def",
        detail: "Ruby method",
        insertText: "def ${1:name}(${2:args})\n\t$0\nend",
        documentation: "Create a Ruby method.",
      },
      {
        label: "class",
        detail: "Ruby class",
        insertText: "class ${1:Name}\n\t$0\nend",
        documentation: "Create a Ruby class.",
      },
    ],
    declarationPatterns: [
      { kind: "function", pattern: /\bdef\s+([A-Za-z_][A-Za-z0-9_!?=]*)/g },
      { kind: "class", pattern: /\b(?:class|module)\s+([A-Za-z_][A-Za-z0-9_:]*)/g },
      { kind: "variable", pattern: /(?:^|[^\w])(@{0,2}[A-Za-z_][A-Za-z0-9_]*)/g },
      { kind: "module", pattern: /\brequire\s+["']([^"']+)["']/g },
    ],
  },
];

const LANGUAGE_ALIASES: Record<string, { extensions: string[]; aliases: string[] }> = {
  csharp: { extensions: [".cs", ".csx", ".csproj"], aliases: ["C#", "csharp"] },
  rust: { extensions: [".rs"], aliases: ["Rust", "rust"] },
  lua: { extensions: [".lua"], aliases: ["Lua", "lua"] },
  ruby: { extensions: [".rb", ".rake", ".ru", ".gemspec"], aliases: ["Ruby", "ruby"] },
};

export function registerSemanticCompletionProviders(monaco: MonacoLike) {
  registerCustomLanguages(monaco);

  for (const config of SEMANTIC_RUNTIME_CONFIGS) {
    const providerId = `${config.languageId}:semantic`;
    if (registeredProviderIds.has(providerId)) continue;

    monaco.languages.registerCompletionItemProvider(config.languageId, {
      triggerCharacters: [".", ":", "@", "_"],
      provideCompletionItems: async (model: MonacoLike, position: MonacoLike) => {
        const range = getCompletionRange(monaco, model, position);
        const context = getRuntimeContext(config.runtimeId);
        const file = getFileContextFromModel(model);
        const suggestions = [
          ...buildKeywordSuggestions(monaco, config, range),
          ...buildSnippetSuggestions(monaco, config, range),
          ...buildDeclarationSuggestions(monaco, config, model, range),
          ...buildOpenModelSuggestions(monaco, config, model, range),
          ...buildZenithBuiltinSuggestions(monaco, config, range),
        ];
        const lspSuggestions =
          file && context.isEnabled
            ? await requestLspCompletions(
                file,
                config.languageId,
                model.getValue(),
                position.lineNumber,
                position.column
              ).catch(() => [])
            : [];

        return {
          suggestions: dedupeSuggestions([
            ...suggestions,
            ...lspSuggestions.map((suggestion) => ({
              label: suggestion.label,
              kind:
                suggestion.kind ??
                monaco.languages.CompletionItemKind.Text,
              insertText: suggestion.insertText || suggestion.label,
              detail: suggestion.detail || "Language Server",
              documentation: suggestion.documentation || undefined,
              range,
              sortText: `0-lsp-${suggestion.label}`,
            })),
          ]).map((suggestion) => ({
            ...suggestion,
            detail: context.isEnabled
              ? suggestion.detail
              : `${suggestion.detail || "Semantic completion"} - runtime disabled`,
            documentation: decorateDocumentation(
              suggestion.documentation,
              context
            ),
          })),
        };
      },
    });

    registeredProviderIds.add(providerId);

    if (!registeredHoverProviderIds.has(providerId)) {
      monaco.languages.registerHoverProvider(config.languageId, {
        provideHover: async (model: MonacoLike, position: MonacoLike) => {
          const file = getFileContextFromModel(model);
          if (!file) return null;

          const hover = await requestLspHover(
            file,
            config.languageId,
            model.getValue(),
            position.lineNumber,
            position.column
          ).catch(() => null);

          if (!hover?.contents) return null;
          return {
            contents: [{ value: hover.contents }],
          };
        },
      });
      registeredHoverProviderIds.add(providerId);
    }

    if (!registeredDefinitionProviderIds.has(providerId)) {
      monaco.languages.registerDefinitionProvider(config.languageId, {
        provideDefinition: async (model: MonacoLike, position: MonacoLike) => {
          const file = getFileContextFromModel(model);
          if (!file) return [];

          const definitions = await requestLspDefinition(
            file,
            config.languageId,
            model.getValue(),
            position.lineNumber,
            position.column
          ).catch(() => []);

          return definitions.map((location) => ({
            uri: monaco.Uri.file(location.filePath),
            range: new monaco.Range(location.line, location.col, location.line, location.col + 1),
          }));
        },
      });
      registeredDefinitionProviderIds.add(providerId);
    }

    if (!registeredReferenceProviderIds.has(providerId)) {
      monaco.languages.registerReferenceProvider(config.languageId, {
        provideReferences: async (model: MonacoLike, position: MonacoLike) => {
          const file = getFileContextFromModel(model);
          if (!file) return [];

          const references = await requestLspReferences(
            file,
            config.languageId,
            model.getValue(),
            position.lineNumber,
            position.column
          ).catch(() => []);

          return references.map((location) => ({
            uri: monaco.Uri.file(location.filePath),
            range: new monaco.Range(location.line, location.col, location.line, location.col + 1),
          }));
        },
      });
      registeredReferenceProviderIds.add(providerId);
    }

    if (!registeredRenameProviderIds.has(providerId)) {
      monaco.languages.registerRenameProvider(config.languageId, {
        provideRenameEdits: async (model: MonacoLike, position: MonacoLike, newName: string) => {
          const file = getFileContextFromModel(model);
          if (!file || !newName.trim()) return null;

          const edit = await requestLspRename(
            file,
            config.languageId,
            model.getValue(),
            position.lineNumber,
            position.column,
            newName
          ).catch(() => null);

          if (!edit) return null;
          await applyWorkspaceEdit(edit);
          return { edits: [] };
        },
      });
      registeredRenameProviderIds.add(providerId);
    }
  }
}

function registerCustomLanguages(monaco: MonacoLike) {
  for (const [languageId, definition] of Object.entries(LANGUAGE_ALIASES)) {
    if (registeredLanguageIds.has(languageId)) continue;

    monaco.languages.register({
      id: languageId,
      extensions: definition.extensions,
      aliases: definition.aliases,
    });

    monaco.languages.setLanguageConfiguration(languageId, {
      comments: {
        lineComment: languageId === "ruby" ? "#" : "//",
      },
      brackets: [
        ["{", "}"],
        ["[", "]"],
        ["(", ")"],
      ],
      autoClosingPairs: [
        { open: "{", close: "}" },
        { open: "[", close: "]" },
        { open: "(", close: ")" },
        { open: '"', close: '"' },
        { open: "'", close: "'" },
      ],
    });

    registeredLanguageIds.add(languageId);
  }
}

function getRuntimeContext(runtimeId: RuntimeProfileId) {
  const settings = useWorkspaceStore.getState().settings;
  const config = settings.extensions?.[runtimeId];
  const profile = getRuntimeProfileForSettingsKey(runtimeId);
  const lspPath = config?.lspPath?.trim() || profile.lspPlaceholder;

  return {
    runtimeId,
    isEnabled: Boolean(config?.isEnabled),
    lspPath,
    lspConfigured: Boolean(config?.lspPath?.trim()),
  };
}

function getCompletionRange(monaco: MonacoLike, model: MonacoLike, position: MonacoLike) {
  const word = model.getWordUntilPosition(position);

  return new monaco.Range(
    position.lineNumber,
    word.startColumn,
    position.lineNumber,
    word.endColumn
  );
}

function buildKeywordSuggestions(monaco: MonacoLike, config: SemanticRuntimeConfig, range: MonacoLike) {
  return config.keywords.map((keyword) => ({
    label: keyword,
    kind: monaco.languages.CompletionItemKind.Keyword,
    insertText: keyword,
    detail: `${config.languageId} keyword`,
    documentation: `Keyword from the ${config.languageId} semantic profile.`,
    range,
    sortText: `2-${keyword}`,
  }));
}

function buildSnippetSuggestions(monaco: MonacoLike, config: SemanticRuntimeConfig, range: MonacoLike) {
  return config.snippets.map((snippet) => ({
    label: snippet.label,
    kind: monaco.languages.CompletionItemKind.Snippet,
    insertText: snippet.insertText,
    insertTextRules: monaco.languages.CompletionItemInsertTextRule.InsertAsSnippet,
    detail: snippet.detail,
    documentation: snippet.documentation,
    range,
    sortText: `1-${snippet.label}`,
  }));
}

function buildDeclarationSuggestions(
  monaco: MonacoLike,
  config: SemanticRuntimeConfig,
  model: MonacoLike,
  range: MonacoLike
) {
  return collectDeclarations(config, model.getValue()).map((symbol) => ({
    label: symbol.name,
    kind: toCompletionKind(monaco, symbol.kind),
    insertText: symbol.name,
    detail: `Local ${symbol.kind}`,
    documentation: `Declared in this file on line ${symbol.line}.`,
    range,
    sortText: `0-${symbol.name}`,
  }));
}

function buildOpenModelSuggestions(
  monaco: MonacoLike,
  config: SemanticRuntimeConfig,
  currentModel: MonacoLike,
  range: MonacoLike
) {
  const currentPath = currentModel.uri?.toString?.() || "";
  const symbols: Array<{ name: string; kind: SemanticDeclarationPattern["kind"]; path: string }> = [];

  for (const model of monaco.editor.getModels()) {
    const modelPath = model.uri?.toString?.() || "";
    if (!modelPath || modelPath === currentPath) continue;
    if (model.getLanguageId?.() !== config.languageId) continue;

    for (const symbol of collectDeclarations(config, model.getValue())) {
      symbols.push({
        name: symbol.name,
        kind: symbol.kind,
        path: formatModelPath(modelPath),
      });
    }
  }

  return symbols.slice(0, 120).map((symbol) => ({
    label: symbol.name,
    kind: toCompletionKind(monaco, symbol.kind),
    insertText: symbol.name,
    detail: `Workspace ${symbol.kind}`,
    documentation: `Discovered from an open ${config.languageId} model.\n\n${symbol.path}`,
    range,
    sortText: `3-${symbol.name}`,
  }));
}

function buildZenithBuiltinSuggestions(monaco: MonacoLike, config: SemanticRuntimeConfig, range: MonacoLike) {
  if (config.languageId !== "zenith") return [];

  return Object.entries(ZENITH_BUILTINS).map(([name, doc]) => ({
    label: name,
    kind: monaco.languages.CompletionItemKind.Function,
    insertText: name,
    detail: doc.signature,
    documentation: `${doc.description}\n\n${doc.example}`,
    range,
    sortText: `0-builtins-${name}`,
  }));
}

function collectDeclarations(config: SemanticRuntimeConfig, content: string) {
  const symbols: Array<{
    name: string;
    kind: SemanticDeclarationPattern["kind"];
    line: number;
  }> = [];

  for (const declaration of config.declarationPatterns) {
    declaration.pattern.lastIndex = 0;

    for (const match of content.matchAll(declaration.pattern)) {
      const rawName = match[1];
      if (!rawName || rawName.length > 80) continue;

      const name = rawName.replace(/[.:]+$/, "");
      if (!/^[A-Za-z_@][$\w@!?=:.-]*$/.test(name)) continue;

      symbols.push({
        name,
        kind: declaration.kind,
        line: content.slice(0, match.index ?? 0).split(/\r?\n/).length,
      });
    }
  }

  return dedupeByName(symbols);
}

function dedupeByName<T extends { name: string }>(items: T[]) {
  const seen = new Set<string>();
  const result: T[] = [];

  for (const item of items) {
    const key = item.name.toLowerCase();
    if (seen.has(key)) continue;

    seen.add(key);
    result.push(item);
  }

  return result;
}

function dedupeSuggestions<T extends { label: string }>(items: T[]) {
  const seen = new Set<string>();
  const result: T[] = [];

  for (const item of items) {
    const key = item.label.toLowerCase();
    if (seen.has(key)) continue;

    seen.add(key);
    result.push(item);
  }

  return result;
}

function toCompletionKind(monaco: MonacoLike, kind: SemanticDeclarationPattern["kind"]) {
  switch (kind) {
    case "function":
      return monaco.languages.CompletionItemKind.Function;
    case "class":
      return monaco.languages.CompletionItemKind.Class;
    case "constant":
      return monaco.languages.CompletionItemKind.Constant;
    case "module":
      return monaco.languages.CompletionItemKind.Module;
    default:
      return monaco.languages.CompletionItemKind.Variable;
  }
}

function decorateDocumentation(documentation: unknown, context: ReturnType<typeof getRuntimeContext>) {
  const lspLine = context.lspConfigured
    ? `LSP configured: \`${context.lspPath}\``
    : `Local semantic provider active. Configure \`${context.lspPath}\` to prepare this runtime for an external LSP bridge.`;

  if (typeof documentation === "string") {
    return `${documentation}\n\n---\n${lspLine}`;
  }

  return lspLine;
}

function formatModelPath(path: string) {
  try {
    return decodeURIComponent(path.replace(/^file:\/+/, ""));
  } catch {
    return path;
  }
}
