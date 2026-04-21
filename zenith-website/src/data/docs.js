import { DOCS_MANIFEST } from "./docs.generated";

export const DOCS_META = DOCS_MANIFEST.site;

const SECTION_TRANSLATIONS = {
  pt: { 
    narrative: 'Narrativa', 
    roadmap: 'Roadmap', 
    learn: 'Aprender', 
    reference: 'Referência'
  },
  en: { 
    narrative: 'Narrative', 
    roadmap: 'Roadmap', 
    learn: 'Learn', 
    reference: 'Reference'
  },
  ja: { 
    narrative: '物語', 
    roadmap: 'ロードマップ', 
    learn: '学習', 
    reference: 'リファレンス'
  },
  es: { 
    narrative: 'Narrativa', 
    roadmap: 'Hoja de ruta', 
    learn: 'Aprender', 
    reference: 'Referencia'
  }
};

const CATEGORY_TRANSLATIONS = {
  pt: { 
    'Narrativa': 'Narrativa',
    'Roadmap': 'Roadmap',
    'Aprender': 'Aprender',
    'Referência': 'Referência',
    'Linguagem': 'Linguagem',
    'API': 'API',
    'Módulos': 'Módulos'
  },
  en: { 
    'Narrativa': 'Narrative',
    'Roadmap': 'Roadmap',
    'Aprender': 'Learn',
    'Referência': 'Reference',
    'Linguagem': 'Language',
    'API': 'API',
    'Módulos': 'Modules'
  },
  ja: { 
    'Narrativa': '物語',
    'Roadmap': 'ロードマップ',
    'Aprender': '学習',
    'Referência': 'リファレンス',
    'Linguagem': '言語',
    'API': 'API',
    'Módulos': 'モジュール'
  },
  es: { 
    'Narrativa': 'Narrativa',
    'Roadmap': 'Hoja de ruta',
    'Aprender': 'Aprender',
    'Referência': 'Referencia',
    'Linguagem': 'Lenguaje',
    'API': 'API',
    'Módulos': 'Módulos'
  }
};

export const getDocsSections = (lang = 'pt') => DOCS_MANIFEST.navigation?.main?.map(({ id, label }) => ({
  id,
  label: SECTION_TRANSLATIONS[lang]?.[id] || label || id,
})) || [];

const enrichDoc = (section, group, doc, lang = 'pt') => ({
  ...doc,
  path: `/docs-content/${doc.file}`,
  section: section.id,
  sectionLabel: SECTION_TRANSLATIONS[lang]?.[section.id] || section.id,
  category: CATEGORY_TRANSLATIONS[lang]?.[group.title] || group.title,
});

export const getDocsStructure = (lang = 'pt') => {
  if (!DOCS_MANIFEST.sections) return {};
  
  return Object.fromEntries(
    Object.entries(DOCS_MANIFEST.sections).map(([sectionId, section]) => [
      sectionId,
      section.pages ? [{
        title: SECTION_TRANSLATIONS[lang]?.[sectionId] || section.title,
        docs: section.pages.map((doc) => ({
          ...doc,
          path: `/docs-content/${doc.file}`,
          section: sectionId,
          sectionLabel: SECTION_TRANSLATIONS[lang]?.[sectionId] || section.title,
          category: SECTION_TRANSLATIONS[lang]?.[sectionId] || section.title,
        })),
      }] : [],
    ])
  );
};

export const getAllDocs = (lang = 'pt') => {
  if (!DOCS_MANIFEST.sections) return [];
  
  return Object.entries(DOCS_MANIFEST.sections).flatMap(([sectionId, section]) =>
    section.pages ? section.pages.map((doc) => ({
      ...doc,
      path: `/docs-content/${doc.file}`,
      section: sectionId,
      sectionLabel: SECTION_TRANSLATIONS[lang]?.[sectionId] || section.title,
      category: SECTION_TRANSLATIONS[lang]?.[sectionId] || section.title,
    })) : []
  );
};

export const findDocByFile = (file, lang = 'pt') => getAllDocs(lang).find((doc) => doc.file === file);

