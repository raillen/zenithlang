import React, { useState, useEffect, useMemo, useRef } from 'react';
import { marked } from 'marked';
import Prism from 'prismjs';
import { motion, AnimatePresence } from 'framer-motion';
import Sidebar from '../components/Sidebar';
import Breadcrumbs from '../components/Breadcrumbs';
import Pagination from '../components/Pagination';
import TableOfContents from '../components/TableOfContents';
import ReadingProgressBar from '../components/ReadingProgressBar';
import { getDocsStructure, getAllDocs } from '../data/docs';
import { DOCS_CONTENT } from '../data/docs.content.generated';
import { useLanguage } from '../contexts/LanguageContext';
import { UI_STRINGS } from '../data/translations';
import 'prismjs/components/prism-lua';
import '../utils/zenith-prism';

const stateLabels = {
  pt: { archived: 'Arquivado', current: 'Atual', deprecated: 'Obsoleto', experimental: 'Experimental', stable: 'Estável', vision: 'Visão' },
  en: { archived: 'Archived', current: 'Current', deprecated: 'Deprecated', experimental: 'Experimental', stable: 'Stable', vision: 'Vision' },
  ja: { archived: 'アーカイブ済み', current: '最新', deprecated: '非推奨', experimental: '実験的', stable: '安定', vision: 'ビジョン' },
  es: { archived: 'Archivado', current: 'Actual', deprecated: 'Obsoleto', experimental: 'Experimental', stable: 'Estable', vision: 'Visión' }
};

const stateStyles = {
  archived: 'bg-neutral/10 text-neutral/60',
  current: 'bg-primary/10 text-primary',
  deprecated: 'bg-amber-100 text-amber-700',
  experimental: 'bg-indigo-100 text-indigo-700',
  stable: 'bg-emerald-100 text-emerald-700',
  vision: 'bg-zinc-200 text-zinc-700',
};

// Configuração do marked para ser mais performante e seguro
marked.setOptions({
  headerIds: true,
  gfm: true,
  breaks: true,
});

const contentCache = new Map();

const DocViewer = ({ section, requestedDoc }) => {
  const { lang } = useLanguage();
  const [currentDoc, setCurrentDoc] = useState(null);
  const [content, setContent] = useState('');
  const [loading, setLoading] = useState(true);
  const [isZenMode, setIsZenMode] = useState(false);
  const [isMobileSidebarOpen, setIsMobileSidebarOpen] = useState(false);
  const articleRef = useRef(null);

  // Achatando estrutura para navegação linear
  const flatDocs = useMemo(() => getAllDocs(lang), [lang]);
  
  const sectionDocs = useMemo(() => 
    flatDocs.filter(d => d.section === section), 
  [section, flatDocs]);

  useEffect(() => {
    if (requestedDoc && requestedDoc.section === section) {
      handleDocSelect(requestedDoc);
    } else {
      const structure = getDocsStructure(lang)[section];
      if (structure && structure.length > 0 && structure[0].docs.length > 0) {
        handleDocSelect(structure[0].docs[0]);
      } else {
        setContent(`<div class="p-8 text-neutral/40 font-medium">${UI_STRINGS[lang].docViewer.development}</div>`);
        setLoading(false);
      }
    }
  }, [section, requestedDoc, lang]);

  const loadDoc = async (file) => {
    const cacheKey = `${lang}:${file}`;
    if (contentCache.has(cacheKey)) {
      setContent(contentCache.get(cacheKey));
      setLoading(false);
      window.scrollTo({ top: 0, behavior: 'instant' });
      return;
    }

    setLoading(true);
    try {
      const langContent = DOCS_CONTENT[lang] || {};
      const text = langContent[file];

      if (typeof text !== 'string') {
        throw new Error(`Document [${lang}] not found: ${file}`);
      }
      
      const cleanMd = text.replace(/^---[\s\S]*?---/, '');
      const htmlContent = marked.parse(cleanMd);
      
      contentCache.set(cacheKey, htmlContent);
      
      setContent(htmlContent);
      window.scrollTo({ top: 0, behavior: 'smooth' });
    } catch (err) {
      console.error("Failed to load document:", err);
      setContent(`<div class="p-8 text-red-500 font-medium">${UI_STRINGS[lang].docViewer.error}</div>`);
    }
    setLoading(false);
  };

  const [pageSymbols, setPageSymbols] = useState([]);

  useEffect(() => {
    if (!content || !articleRef.current) return;

    const symbols = [];
    const container = articleRef.current;
    
    const headers = container.querySelectorAll('h2, h3');
    headers.forEach(header => {
      if (!header.id) {
        header.id = header.innerText.toLowerCase().replace(/[^\w\s-]/g, '').replace(/\s+/g, '-');
      }
      symbols.push({
        id: header.id,
        label: header.innerText,
        type: header.tagName.toLowerCase() === 'h2' ? 'section' : 'subsection',
        top: header.offsetTop
      });
    });

    const tables = container.querySelectorAll('table');
    tables.forEach(table => {
      const ths = Array.from(table.querySelectorAll('th'));
      const isApiTable = ths.some(th => {
        const text = th.innerText.toLowerCase();
        return text.includes('api') || text.includes('função') || text.includes('método');
      });

      if (isApiTable) {
        const rows = table.querySelectorAll('tbody tr');
        rows.forEach((row, idx) => {
          const firstCell = row.querySelector('td');
          if (firstCell) {
            const apiText = firstCell.innerText.trim();
            const cleanId = apiText.split('(')[0].toLowerCase().replace(/[^\w\s-]/g, '').replace(/\s+/g, '-');
            const uniqueId = `api-${cleanId}`;
            
            row.id = uniqueId;
            row.classList.add('api-symbol-row', 'transition-colors', 'duration-500');
            
            symbols.push({
              id: uniqueId,
              label: apiText,
              type: 'function',
              top: row.offsetTop
            });
          }
        });
      }
    });

    setPageSymbols(symbols);

    const hash = window.location.hash.substring(1);
    if (hash) {
      setTimeout(() => {
        const element = container.querySelector(`#${hash}`);
        if (element) {
          const offset = 100;
          const rect = element.getBoundingClientRect();
          const offsetPosition = rect.top + window.pageYOffset - offset;
          window.scrollTo({ top: offsetPosition, behavior: 'smooth' });
          element.classList.add('bg-primary/5', 'ring-2', 'ring-primary/20');
          setTimeout(() => element.classList.remove('bg-primary/5', 'ring-2', 'ring-primary/20'), 3000);
        }
      }, 500);
    }

    Prism.highlightAllUnder(container);

    const preBlocks = container.querySelectorAll('pre');
    preBlocks.forEach(pre => {
      if (pre.querySelector('.copy-btn-vanilla')) return;
      
      pre.style.position = 'relative';
      const btn = document.createElement('button');
      btn.className = 'copy-btn-vanilla absolute top-3 right-3 p-2 rounded-lg bg-white/80 backdrop-blur-sm border border-black/5 text-neutral/40 hover:text-primary transition-all z-10 cursor-pointer flex items-center justify-center gap-1 opacity-0 group-hover:opacity-100';
      btn.innerHTML = '<svg width="14" height="14" viewBox="0 0 256 256"><path fill="currentColor" d="M216,40H88a16,16,0,0,0-16,16V72H56A16,16,0,0,0,40,88V216a16,16,0,0,0,16,16H184a16,16,0,0,0,16-16V200h16a16,16,0,0,0,16-16V56A16,16,0,0,0,216,40ZM184,216H56V88H184V216Zm32-32H200V88a16,16,0,0,0-16-16H88V56H216V184Z"></path></svg>';
      btn.title = 'Copiar código';
      
      pre.classList.add('group');
      
      btn.onclick = () => {
        const code = pre.querySelector('code')?.innerText || pre.innerText;
        navigator.clipboard.writeText(code).then(() => {
          btn.innerHTML = '<svg width="14" height="14" viewBox="0 0 256 256" class="text-emerald-500"><path fill="currentColor" d="M229.66,77.66l-128,128a8,8,0,0,1-11.32,0l-56-56a8,8,0,0,1,11.32-11.32L96,188.69,218.34,66.34a8,8,0,0,1,11.32,11.32Z"></path></svg>';
          setTimeout(() => {
            btn.innerHTML = '<svg width="14" height="14" viewBox="0 0 256 256"><path fill="currentColor" d="M216,40H88a16,16,0,0,0-16,16V72H56A16,16,0,0,0,40,88V216a16,16,0,0,0,16,16H184a16,16,0,0,0,16-16V200h16a16,16,0,0,0,16-16V56A16,16,0,0,0,216,40ZM184,216H56V88H184V216Zm32-32H200V88a16,16,0,0,0-16-16H88V56H216V184Z"></path></svg>';
          }, 2000);
        });
      };
      
      pre.appendChild(btn);
    });
  }, [content]);

  useEffect(() => {
    const handleEsc = (e) => {
      if (e.key === 'Escape' && isMobileSidebarOpen) {
        setIsMobileSidebarOpen(false);
      }
    };
    window.addEventListener('keydown', handleEsc);
    return () => window.removeEventListener('keydown', handleEsc);
  }, [isMobileSidebarOpen]);

  const scrollToSymbol = (id) => {
    const element = document.getElementById(id);
    if (element) {
      const offset = 100;
      const elementPosition = element.getBoundingClientRect().top;
      const offsetPosition = elementPosition + window.pageYOffset - offset;

      window.scrollTo({
        top: offsetPosition,
        behavior: 'smooth'
      });

      element.classList.add('bg-primary/5', 'ring-2', 'ring-primary/20');
      setTimeout(() => {
        element.classList.remove('bg-primary/5', 'ring-2', 'ring-primary/20');
      }, 2000);
    }
  };

  const handleDocSelect = (doc) => {
    setCurrentDoc(doc);
    loadDoc(doc.file);
    setIsMobileSidebarOpen(false);
  };

  const currentIndex = sectionDocs.findIndex(d => d.path === currentDoc?.path);
  const prevDoc = currentIndex > 0 ? sectionDocs[currentIndex - 1] : null;
  const nextDoc = currentIndex < sectionDocs.length - 1 ? sectionDocs[currentIndex + 1] : null;

  return (
    <div data-z-id="docviewer-page" className="page-doc-viewer flex w-full bg-[#ECEEEE] transition-all duration-500 relative" role="region" aria-label="Visualizador de Documentação">
      <ReadingProgressBar />
      
      <AnimatePresence>
        {isMobileSidebarOpen && (
          <>
            <motion.div 
              initial={{ opacity: 0 }}
              animate={{ opacity: 1 }}
              exit={{ opacity: 0 }}
              onClick={() => setIsMobileSidebarOpen(false)}
              className="fixed inset-0 bg-black/20 backdrop-blur-sm z-[60] lg:hidden"
              aria-hidden="true"
            />
            <motion.div
              initial={{ x: -280 }}
              animate={{ x: 0 }}
              exit={{ x: -280 }}
              transition={{ type: "spring", damping: 25, stiffness: 200 }}
              className="fixed left-0 top-0 bottom-0 w-[280px] bg-white z-[70] lg:hidden shadow-2xl"
              role="dialog"
              aria-modal="true"
              aria-label="Menu de tópicos da documentação"
            >
              <Sidebar 
                sections={getDocsStructure(lang)[section] || []} 
                currentDoc={currentDoc}
                onDocSelect={handleDocSelect}
                isMobile={true}
              />
            </motion.div>
          </>
        )}
      </AnimatePresence>

      {!isZenMode && (
        <aside 
          data-z-id="docviewer-sidebar-wrapper" 
          className="doc-sidebar shrink-0 hidden lg:block"
          aria-label="Navegação lateral"
        >
          <Sidebar 
            sections={getDocsStructure(lang)[section] || []} 
            currentDoc={currentDoc}
            onDocSelect={handleDocSelect}
          />
        </aside>
      )}
      
      <main data-z-id="docviewer-main-content" className={`doc-content flex-1 px-6 py-10 md:px-16 md:py-16 mx-auto min-h-[calc(100vh-64px)] relative transition-all duration-500 w-full ${isZenMode ? 'max-w-3xl' : 'max-w-4xl'}`}>
        <button
          onClick={() => setIsMobileSidebarOpen(true)}
          className="lg:hidden fixed bottom-8 right-8 z-50 bg-[#111111] text-white p-4 rounded-full shadow-2xl flex items-center gap-2 hover:scale-105 active:scale-95 transition-all focus-visible:ring-4 focus-visible:ring-primary/40 outline-none"
          aria-label="Abrir tópicos da documentação"
          aria-expanded={isMobileSidebarOpen}
        >
          <svg xmlns="http://www.w3.org/2000/svg" width="20" height="20" fill="currentColor" viewBox="0 0 256 256" aria-hidden="true"><path d="M224,128a8,8,0,0,1-8,8H40a8,8,0,0,1,0-16H216A8,8,0,0,1,224,128ZM40,72H216a8,8,0,0,0,0-16H40a8,8,0,0,0,0-16ZM216,184H40a8,8,0,0,0,0,16H216a8,8,0,0,0,0-16Z"></path></svg>
          <span className="text-sm font-bold pr-2">{UI_STRINGS[lang].nav.topics}</span>
        </button>

        <header data-z-id="docviewer-breadcrumbs-wrapper" className="doc-breadcrumbs mb-8 flex flex-col sm:flex-row items-start sm:items-center justify-between gap-4">
          <Breadcrumbs 
            section={section} 
            category={currentDoc?.category} 
            label={currentDoc?.label} 
            symbols={pageSymbols}
            onSymbolSelect={scrollToSymbol}
          />
          
          <motion.button 
            whileHover={{ scale: 1.05 }}
            whileTap={{ scale: 0.95 }}
            onClick={() => setIsZenMode(!isZenMode)}
            aria-label={isZenMode ? UI_STRINGS[lang].nav.zenModeOn : UI_STRINGS[lang].nav.zenModeOff}
            aria-pressed={isZenMode}
            className={`px-4 py-2 rounded-full border transition-all flex items-center gap-2 text-[10px] font-bold uppercase tracking-widest outline-none focus-visible:ring-2 focus-visible:ring-primary ${isZenMode ? 'bg-primary text-white border-primary shadow-lg shadow-primary/20' : 'bg-white border-black/5 text-neutral/40 hover:text-primary hover:border-primary/20'}`}
          >
            {isZenMode ? UI_STRINGS[lang].nav.zenModeOn : UI_STRINGS[lang].nav.zenModeOff}
          </motion.button>
        </header>

        {currentDoc && (
          <header className="mb-12">
            <div className="mb-4 flex flex-wrap items-center gap-x-3 gap-y-2">
              <span className={`text-[9px] font-black uppercase tracking-widest ${
                currentDoc.state === 'current' ? 'text-primary' : 
                currentDoc.state === 'stable' ? 'text-emerald-600' :
                'text-neutral/40'
              }`}>
                {stateLabels[lang]?.[currentDoc.state] || currentDoc.state || 'Doc'}
              </span>
              <span className="h-3 w-px bg-black/10" />
              <span className="text-[9px] font-bold uppercase tracking-widest text-neutral/30">
                {currentDoc.category}
              </span>
              <span className="h-3 w-px bg-black/10" />
              <span className="text-[9px] font-bold uppercase tracking-widest text-neutral/30">
                {currentDoc.sectionLabel}
              </span>
            </div>
            <h1 className="text-4xl font-black tracking-tight text-gray-900 md:text-6xl">
              {currentDoc.label}
            </h1>
            {currentDoc.summary && (
              <p className="mt-6 max-w-[60ch] text-xl leading-relaxed text-gray-500/80 font-medium">
                {currentDoc.summary}
              </p>
            )}
          </header>
        )}

        {loading ? (
          <div data-z-id="docviewer-loading-skeleton" className="doc-loading flex flex-col gap-6 animate-pulse" aria-hidden="true">
            <div data-z-id="docviewer-skeleton-1" className="h-10 bg-black/5 rounded-xl w-3/4" />
            <div data-z-id="docviewer-skeleton-2" className="h-4 bg-black/5 rounded-lg w-full" />
            <div data-z-id="docviewer-skeleton-3" className="h-4 bg-black/5 rounded-lg w-5/6" />
            <div data-z-id="docviewer-skeleton-4" className="h-4 bg-black/5 rounded-lg w-full" />
            <div data-z-id="docviewer-skeleton-5" className="h-64 bg-black/5 rounded-2xl w-full mt-8" />
          </div>
        ) : (
          <AnimatePresence mode="wait">
            <motion.article 
              key={currentDoc?.path + lang}
              initial={{ opacity: 0, y: 10 }}
              animate={{ opacity: 1, y: 0 }}
              exit={{ opacity: 0, y: -10 }}
              transition={{ duration: 0.3, ease: "easeOut" }}
              data-z-id="docviewer-article"
              ref={articleRef}
              className="doc-article prose prose-slate prose-lg max-w-none 
                         prose-headings:font-display prose-headings:text-neutral
                         prose-h1:hidden
                         prose-pre:bg-[#FAFAFA] prose-pre:border prose-pre:border-black/5 prose-pre:shadow-sm
                         prose-a:text-primary hover:prose-a:text-primary-hover transition-colors
                         prose-img:rounded-3xl prose-blockquote:rounded-r-xl w-full overflow-x-hidden"
              dangerouslySetInnerHTML={{ __html: content }} 
            />
            
            <nav 
              key={`nav-${currentDoc?.path}`}
              className="doc-pagination mt-20 pt-8 border-t border-black/10"
              aria-label="Páginas anterior e próxima"
            >
              <Pagination 
                prev={prevDoc} 
                next={nextDoc} 
                onNavigate={handleDocSelect} 
              />
            </nav>
          </AnimatePresence>
        )}
      </main>

      {!isZenMode && (
        <aside 
          data-z-id="docviewer-toc-wrapper" 
          className="doc-toc shrink-0 hidden xl:block w-64"
          aria-label="Sumário da página"
        >
          <TableOfContents 
            content={content} 
            symbols={pageSymbols}
            onSymbolSelect={scrollToSymbol} 
          />
        </aside>
      )}
    </div>
  );
};

export default DocViewer;
