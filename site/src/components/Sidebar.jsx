import React from 'react';
import { motion } from 'framer-motion';
import { 
  BookOpen, 
  Wrench, 
  Package, 
  GraduationCap, 
  Desktop, 
  Gear, 
  RocketLaunch, 
  House, 
  Scroll,
  FileText
} from '@phosphor-icons/react';
import { useLanguage } from '../contexts/LanguageContext';

const categoryIcons = {
  'Essenciais': <House weight="duotone" className="w-4 h-4" />,
  'Estado da Linguagem': <BookOpen weight="duotone" className="w-4 h-4" />,
  'Linhas do Compilador': <Gear weight="duotone" className="w-4 h-4" />,
  'Linhas de Versão': <Scroll weight="duotone" className="w-4 h-4" />,
  'Implementação': <RocketLaunch weight="duotone" className="w-4 h-4" />,
  'Biblioteca Padrão': <Package weight="duotone" className="w-4 h-4" />,
  'Handbook': <GraduationCap weight="duotone" className="w-4 h-4" />,
  'Especificação': <Desktop weight="duotone" className="w-4 h-4" />,
  'Módulos Oficiais': <Package weight="duotone" className="w-4 h-4" />,
  'Leitura e Diagnóstico': <Wrench weight="duotone" className="w-4 h-4" />
};

const stateStyles = {
  archived: 'bg-neutral/10 text-neutral/50',
  current: 'bg-primary/10 text-primary',
  deprecated: 'bg-amber-100 text-amber-700',
  experimental: 'bg-indigo-100 text-indigo-700',
  stable: 'bg-emerald-100 text-emerald-700',
  vision: 'bg-zinc-200 text-zinc-700',
};

const Sidebar = ({ sections, currentDoc, onDocSelect, isMobile = false }) => {
  const { lang } = useLanguage();
  
  const searchLabels = {
    pt: 'Buscar...',
    en: 'Search...',
    ja: '検索...',
    es: 'Buscar...'
  };

  // Re-mapear ícones para suportar tradução de títulos
  const getIcon = (title) => {
    const CATEGORY_TRANSLATIONS = {
      pt: { 'Essenciais': 'Essenciais', 'Estado da Linguagem': 'Estado da Linguagem', 'Linhas do Compilador': 'Linhas do Compilador', 'Linhas de Versão': 'Linhas de Versão', 'Implementação': 'Implementação', 'Handbook': 'Handbook', 'Especificacao': 'Especificação', 'Módulos Oficiais': 'Módulos Oficiais', 'Leitura e Diagnostico': 'Leitura e Diagnóstico' },
      en: { 'Essenciais': 'Essentials', 'Estado da Linguagem': 'Language State', 'Linhas do Compilador': 'Compiler Lines', 'Linhas de Versão': 'Version Lines', 'Implementação': 'Implementation', 'Handbook': 'Handbook', 'Especificacao': 'Specification', 'Módulos Oficiais': 'Official Modules', 'Leitura e Diagnostico': 'Reading & Diagnostics' },
      ja: { 'Essenciais': 'エッセンシャル', 'Estado da Linguagem': '言語の状態', 'Linhas do Compilador': 'コンパイラライン', 'Linhas de Versão': 'バージョンライン', 'Implementação': '実装', 'Handbook': 'ハンドブック', 'Especificacao': '仕様', 'Módulos Oficiais': '公式モジュール', 'Leitura e Diagnostico': '読み取りと診断' },
      es: { 'Essenciais': 'Esenciales', 'Estado da Linguagem': 'Estado del lenguaje', 'Linhas do Compilador': 'Líneas del compilador', 'Linhas de Versão': 'Líneas de versión', 'Implementação': 'Implementación', 'Handbook': 'Manual', 'Especificacao': 'Especificación', 'Módulos Oficiais': 'Módulos oficiales', 'Leitura e Diagnostico': 'Lectura y diagnóstico' }
    };

    const currentLangTranslations = CATEGORY_TRANSLATIONS[lang] || {};
    
    let originalKey = title;
    for (const [key, value] of Object.entries(currentLangTranslations)) {
      if (value === title) {
        originalKey = key;
        break;
      }
    }

    return categoryIcons[originalKey] || categoryIcons[title] || <FileText weight="duotone" className="w-4 h-4" />;
  };

  return (
    <aside 
      data-z-id="sidebar-container" 
      className={`doc-sidebar overflow-y-auto px-6 py-10 transition-all ${
        isMobile 
          ? 'w-full h-full pb-32' 
          : 'w-64 h-[calc(100vh-64px)] glass-premium border-r border-black/5 sticky top-16'
      }`}
    >
      {/* Search Trigger */}
      <button 
        data-z-id="sidebar-search-trigger"
        onClick={() => window.dispatchEvent(new KeyboardEvent('keydown', { ctrlKey: true, key: 'k' }))}
        className="sidebar-search-trigger w-full mb-8 p-3 rounded-xl border border-black/5 flex items-center justify-between text-neutral/40 hover:border-primary/20 hover:bg-primary/5 transition-all group"
        aria-label="Buscar documentação (Ctrl+K)"
      >
        <div data-z-id="sidebar-search-trigger-content" className="flex items-center gap-2">
          <svg data-z-id="sidebar-search-trigger-icon" xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={2} stroke="currentColor" className="w-4 h-4 group-hover:text-primary transition-colors" aria-hidden="true">
            <path strokeLinecap="round" strokeLinejoin="round" d="M21 21l-5.197-5.197m0 0A7.5 7.5 0 105.196 5.196a7.5 7.5 0 0010.607 10.607z" />
          </svg>
          <span data-z-id="sidebar-search-trigger-text" className="text-sm font-medium">{searchLabels[lang]}</span>
        </div>
        <div data-z-id="sidebar-search-trigger-hint" className="sidebar-search-hint px-2 py-1 rounded border border-black/10 text-xs font-semibold bg-white/80 shadow-sm">Ctrl K</div>
      </button>

      <nav data-z-id="sidebar-nav" className="sidebar-nav flex flex-col gap-8" aria-label="Navegação da documentação">
        {sections.map((section) => (
          <div data-z-id={`sidebar-category-${section.title.replace(/\s+/g, '-').toLowerCase()}`} key={section.title} className="sidebar-category flex flex-col gap-2">
            <h4 data-z-id="sidebar-category-title" className="sidebar-category-title px-3 text-xs font-bold uppercase tracking-wider text-neutral/50 mb-1 flex items-center gap-2">
              <span data-z-id="sidebar-category-icon" aria-hidden="true" className="flex items-center justify-center text-neutral/50">
                {getIcon(section.title)}
              </span>
              {section.title}
            </h4>
            <ul data-z-id="sidebar-doc-list" className="sidebar-doc-list flex flex-col gap-1" role="list">
              {section.docs.map((doc) => (
                <li data-z-id={`sidebar-doc-item-${doc.path.replace(/[\/\.]/g, '-')}`} key={doc.path}>
                  <button
                    data-z-id="sidebar-doc-link"
                    onClick={() => onDocSelect(doc)}
                    className={`sidebar-doc-link w-full text-left px-3 py-2.5 text-sm font-medium rounded-lg transition-all flex items-center justify-between group ${
                      currentDoc?.path === doc.path
                        ? 'bg-primary/10 text-primary'
                        : 'text-neutral/60 hover:text-neutral hover:bg-black/5'
                    }`}
                    aria-current={currentDoc?.path === doc.path ? 'page' : undefined}
                  >
                    <span data-z-id="sidebar-doc-label" className="min-w-0 pr-3">{doc.label}</span>
                    <div className="flex items-center gap-2 shrink-0">
                      {doc.state && (
                        <span className={`rounded-full px-2 py-1 text-[10px] font-bold uppercase tracking-wide ${stateStyles[doc.state] || 'bg-neutral/10 text-neutral/60'}`}>
                          {doc.state}
                        </span>
                      )}
                      {currentDoc?.path === doc.path && (
                        <motion.div data-z-id="sidebar-doc-active-indicator" layoutId="active-pill" className="w-1.5 h-1.5 rounded-full bg-primary" aria-hidden="true" />
                      )}
                    </div>
                  </button>
                </li>
              ))}
            </ul>
          </div>
        ))}
      </nav>
    </aside>
  );
};

export default Sidebar;
