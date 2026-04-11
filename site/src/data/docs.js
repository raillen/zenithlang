import { DOCS_MANIFEST } from "./docs.generated";

export const DOCS_META = DOCS_MANIFEST.site;

const SECTION_TRANSLATIONS = {
  pt: { start: 'Visão & Marcas', language: 'Linguagem', compiler: 'Compiladores', versions: 'Versões', roadmap: 'Roadmap', learn: 'Aprender', reference: 'Referência', support: 'Suporte' },
  en: { start: 'Vision & Brands', language: 'Language', compiler: 'Compilers', versions: 'Versions', roadmap: 'Roadmap', learn: 'Learn', reference: 'Reference', support: 'Support' },
  ja: { start: 'ビジョンとブランド', language: '言語', compiler: 'コンパイラ', versions: 'バージョン', roadmap: 'ロードマップ', learn: '学習', reference: 'リファレンス', support: 'サポート' },
  es: { start: 'Visión y Marcas', language: 'Lenguaje', compiler: 'Compiladores', versions: 'Versiones', roadmap: 'Plan de ruta', learn: 'Aprender', reference: 'Referencia', support: 'Soporte' }
};

const CATEGORY_TRANSLATIONS = {
  pt: { 'Essenciais': 'Essenciais', 'Estado da Linguagem': 'Estado da Linguagem', 'Linhas do Compilador': 'Linhas do Compilador', 'Linhas de Versão': 'Linhas de Versão', 'Implementação': 'Implementação', 'Handbook': 'Handbook', 'Especificacao': 'Especificação', 'Módulos Oficiais': 'Módulos Oficiais', 'Leitura e Diagnostico': 'Leitura e Diagnóstico' },
  en: { 'Essenciais': 'Essentials', 'Estado da Linguagem': 'Language State', 'Linhas do Compilador': 'Compiler Lines', 'Linhas de Versão': 'Version Lines', 'Implementação': 'Implementation', 'Handbook': 'Handbook', 'Especificacao': 'Specification', 'Módulos Oficiais': 'Official Modules', 'Leitura e Diagnostico': 'Reading & Diagnostics' },
  ja: { 'Essenciais': 'エッセンシャル', 'Estado da Linguagem': '言語の状態', 'Linhas do Compilador': 'コンパイラライン', 'Linhas de Versão': 'バージョンライン', 'Implementação': '実装', 'Handbook': 'ハンドブック', 'Especificacao': '仕様', 'Módulos Oficiais': '公式モジュール', 'Leitura e Diagnostico': '読み取りと診断' },
  es: { 'Essenciais': 'Esenciales', 'Estado da Linguagem': 'Estado del lenguaje', 'Linhas do Compilador': 'Líneas del compilador', 'Linhas de Versão': 'Líneas de versión', 'Implementação': 'Implementación', 'Handbook': 'Manual', 'Especificacao': 'Especificación', 'Módulos Oficiais': 'Módulos oficiales', 'Leitura e Diagnostico': 'Lectura y diagnóstico' }
};

export const getDocsSections = (lang = 'pt') => DOCS_MANIFEST.navigation.map(({ id }) => ({
  id,
  label: SECTION_TRANSLATIONS[lang]?.[id] || id,
}));

const enrichDoc = (section, group, doc, lang = 'pt') => ({
  ...doc,
  path: `/docs-content/${doc.file}`,
  section: section.id,
  sectionLabel: SECTION_TRANSLATIONS[lang]?.[section.id] || section.id,
  category: CATEGORY_TRANSLATIONS[lang]?.[group.title] || group.title,
});

export const getDocsStructure = (lang = 'pt') => Object.fromEntries(
  DOCS_MANIFEST.navigation.map((section) => [
    section.id,
    section.groups.map((group) => ({
      title: CATEGORY_TRANSLATIONS[lang]?.[group.title] || group.title,
      docs: group.docs.map((doc) => enrichDoc(section, group, doc, lang)),
    })),
  ]),
);

export const getAllDocs = (lang = 'pt') => DOCS_MANIFEST.navigation.flatMap((section) =>
  section.groups.flatMap((group) => group.docs.map((doc) => enrichDoc(section, group, doc, lang))),
);

export const findDocByFile = (file, lang = 'pt') => getAllDocs(lang).find((doc) => doc.file === file);
