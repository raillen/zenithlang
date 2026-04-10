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

const categoryIcons = {
  'Guia do Usuário': <BookOpen weight="duotone" className="w-4 h-4" />,
  'Suporte': <Wrench weight="duotone" className="w-4 h-4" />,
  'Biblioteca Padrão': <Package weight="duotone" className="w-4 h-4" />,
  'Teoria': <GraduationCap weight="duotone" className="w-4 h-4" />,
  'Implementação': <Desktop weight="duotone" className="w-4 h-4" />,
  'Sistema': <Gear weight="duotone" className="w-4 h-4" />,
  'Desenvolvimento': <RocketLaunch weight="duotone" className="w-4 h-4" />,
  'Projeto': <House weight="duotone" className="w-4 h-4" />,
  'Histórico': <Scroll weight="duotone" className="w-4 h-4" />
};

const Sidebar = ({ sections, currentDoc, onDocSelect, isMobile = false }) => {
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
          <span data-z-id="sidebar-search-trigger-text" className="text-sm font-medium">Buscar...</span>
        </div>
        <div data-z-id="sidebar-search-trigger-hint" className="sidebar-search-hint px-2 py-1 rounded border border-black/10 text-xs font-semibold bg-white/80 shadow-sm">Ctrl K</div>
      </button>

      <nav data-z-id="sidebar-nav" className="sidebar-nav flex flex-col gap-8" aria-label="Navegação da documentação">
        {sections.map((section) => (
          <div data-z-id={`sidebar-category-${section.title.replace(/\s+/g, '-').toLowerCase()}`} key={section.title} className="sidebar-category flex flex-col gap-2">
            <h4 data-z-id="sidebar-category-title" className="sidebar-category-title px-3 text-xs font-bold uppercase tracking-wider text-neutral/50 mb-1 flex items-center gap-2">
              <span data-z-id="sidebar-category-icon" aria-hidden="true" className="flex items-center justify-center text-neutral/50">
                {categoryIcons[section.title] || <FileText weight="duotone" className="w-4 h-4" />}
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
                    <span data-z-id="sidebar-doc-label">{doc.label}</span>
                    {currentDoc?.path === doc.path && (
                      <motion.div data-z-id="sidebar-doc-active-indicator" layoutId="active-pill" className="w-1.5 h-1.5 rounded-full bg-primary" aria-hidden="true" />
                    )}
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