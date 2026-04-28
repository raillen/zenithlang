import { LanguageSupport, StreamLanguage, StringStream } from "@codemirror/language";

const KEYWORDS = [
  "fn", "let", "var", "const", "if", "else", "for", "while", "loop",
  "return", "break", "continue", "match", "struct", "enum", "impl",
  "trait", "pub", "mut", "self", "super", "import", "from", "as",
  "true", "false", "nil", "in", "is", "not", "and", "or", "where",
  "type", "interface", "class", "new", "this", "static", "async", "await",
  "try", "catch", "throw", "module", "export", "use",
];

const TYPES = [
  "int", "float", "bool", "string", "void", "any", "list", "map",
  "optional", "result", "vec2", "vec3", "vec4", "entity", "component",
];

const BUILTINS = [
  "print", "println", "len", "push", "pop", "append", "remove",
  "contains", "keys", "values", "to_string", "parse",
  "abs", "min", "max", "clamp", "floor", "ceil", "round", "sqrt",
  "sin", "cos", "tan", "atan2", "lerp", "random",
];

const keywordSet = new Set(KEYWORDS);
const typeSet = new Set(TYPES);
const builtinSet = new Set(BUILTINS);

const zenithStreamParser = {
  startState(): { inBlockComment: boolean } {
    return { inBlockComment: false };
  },

  token(stream: StringStream, state: { inBlockComment: boolean }): string | null {
    if (state.inBlockComment) {
      if (stream.match("*/")) {
        state.inBlockComment = false;
        return "blockComment";
      }
      stream.next();
      return "blockComment";
    }

    if (stream.match("/*")) {
      state.inBlockComment = true;
      return "blockComment";
    }

    if (stream.match("//")) {
      stream.skipToEnd();
      return "lineComment";
    }

    if (stream.match(/^"(?:[^"\\]|\\.)*"/)) return "string";
    if (stream.match(/^'(?:[^'\\]|\\.)*'/)) return "string";

    if (stream.match(/^0x[0-9a-fA-F_]+/)) return "number";
    if (stream.match(/^0b[01_]+/)) return "number";
    if (stream.match(/^[0-9][0-9_]*(?:\.[0-9_]+)?(?:[eE][+-]?[0-9]+)?/)) return "number";

    if (stream.match(/^@[a-zA-Z_][a-zA-Z0-9_]*/)) return "meta";

    if (stream.match(/^[a-zA-Z_][a-zA-Z0-9_]*/)) {
      const word = stream.current();
      if (keywordSet.has(word)) return "keyword";
      if (typeSet.has(word)) return "typeName";
      if (builtinSet.has(word)) return "variableName.standard";
      if (word[0] === word[0].toUpperCase() && /^[A-Z]/.test(word)) return "typeName";
      return "variableName";
    }

    if (stream.match(/^[+\-*/%=<>!&|^~?.,:;{}()\[\]@#]+/)) return "operator";

    stream.next();
    return null;
  },
};

export const zenithLanguage = StreamLanguage.define(zenithStreamParser);

export function zenithLangSupport(): LanguageSupport {
  return new LanguageSupport(zenithLanguage);
}
