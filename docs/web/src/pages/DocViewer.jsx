import React, { useState, useEffect, useMemo, useRef } from 'react';
import { marked } from 'marked';
import Prism from 'prismjs';
import { Copy, Check } from '@phosphor-icons/react';
import { createRoot } from 'react-dom/client';
import { motion, AnimatePresence } from 'framer-motion';
import Sidebar from '../components/Sidebar';
import Breadcrumbs from '../components/Breadcrumbs';
import Pagination from '../components/Pagination';
import TableOfContents from '../components/TableOfContents';
import ReadingProgressBar from '../components/ReadingProgressBar';
import { DOCS_STRUCTURE, getAllDocs } from '../data/docs';
import 'prismjs/components/prism-lua';

// Configurar Marked para gerar IDs nos cabeçalhos
const renderer = new marked.Renderer();
renderer.heading = ({ text, depth }) => {
  const id = String(text).toLowerCase().replace(/[^\w\s-]/g, '').replace(/\s+/g, '-');
  return `<h${depth} id="${id}">${text}</h${depth}>`;
};
marked.setOptions({ renderer });

const CopyButton = ({ textToCopy }) => {
  const [copied, setCopied] = useState(false);

  const handleCopy = () => {
    navigator.clipboard.writeText(textToCopy);
    setCopied(true);
    setTimeout(() => setCopied(false), 2000);
  };

  return (
    <button 
      data-z-id="docviewer-copy-button"
      onClick={handleCopy}
      className="absolute top-3 right-3 p-2 rounded-lg bg-white border border-black/5 text-neutral/40 hover:text-primary hover:shadow-sm hover:-translate-y-0.5 transition-all z-10 cursor-pointer flex items-center justify-center gap-1"
      aria-label="Copiar código"
      title="Copiar para área de transferência"
    >
      {copied ? <Check data-z-id="docviewer-copy-icon-success" size={14} className="text-emerald-500" weight="bold" /> : <Copy data-z-id="docviewer-copy-icon-default" size={14} weight="regular" />}
    </button>
  );
};

// Cache global para evitar re-fetch e re-parse
const contentCache = new Map();

const DocViewer = ({ section, requestedDoc }) => {
  const [currentDoc, setCurrentDoc] = useState(null);
  const [content, setContent] = useState('');
  const [loading, setLoading] = useState(true);
  const [isZenMode, setIsZenMode] = useState(false);
  const [isMobileSidebarOpen, setIsMobileSidebarOpen] = useState(false);
  const articleRef = useRef(null);

  // Achatando estrutura para navegação linear
  const flatDocs = useMemo(() => getAllDocs(), []);
  const sectionDocs = useMemo(() => 
    flatDocs.filter(d => d.section === section), 
  [section, flatDocs]);

  useEffect(() => {
    if (requestedDoc && requestedDoc.section === section) {
      handleDocSelect(requestedDoc);
    } else {
      const structure = DOCS_STRUCTURE[section];
      if (structure && structure.length > 0 && structure[0].docs.length > 0) {
        handleDocSelect(structure[0].docs[0]);
      } else {
        setContent('<div class="p-8 text-neutral/40 font-medium">Seção em desenvolvimento.</div>');
        setLoading(false);
      }
    }
  }, [section, requestedDoc]);

  const loadDoc = async (path) => {
    // Verificar cache primeiro
    if (contentCache.has(path)) {
      setContent(contentCache.get(path));
      setLoading(false);
      window.scrollTo({ top: 0, behavior: 'instant' });
      return;
    }

    setLoading(true);
    try {
      const baseUrl = import.meta.env.BASE_URL === '/' ? '' : import.meta.env.BASE_URL;
      const fetchPath = path.startsWith('/') ? `${baseUrl}${path}` : `${baseUrl}/${path}`;
      
      const response = await fetch(fetchPath);
      
      if (!response.ok) {
        throw new Error(`Erro HTTP: ${response.status}`);
      }
      
      const text = await response.text();
      
      if (text.trim().startsWith('<!doctype html>') || text.trim().startsWith('<!DOCTYPE html>')) {
        throw new Error('Recebeu HTML ao invés de Markdown (404 Fallback)');
      }
      
      const cleanMd = text.replace(/^---[\s\S]*?---/, '');
      const htmlContent = marked.parse(cleanMd);
      
      // Salvar no cache
      contentCache.set(path, htmlContent);
      
      setContent(htmlContent);
      window.scrollTo({ top: 0, behavior: 'smooth' });
    } catch (err) {
      console.error("Failed to load document:", err);
      setContent('<div class="p-8 text-red-500 font-medium">Erro ao carregar documento. Verifique se o arquivo existe.</div>');
    }
    setLoading(false);
  };

  // Delegar eventos de cópia para evitar múltiplos roots do React
  useEffect(() => {
    if (!content || !articleRef.current) return;

    // Highlight syntax
    Prism.highlightAllUnder(articleRef.current);

    // Injetar botões de cópia de forma leve (sem createRoot)
    const preBlocks = articleRef.current.querySelectorAll('pre');
    preBlocks.forEach(pre => {
      if (pre.querySelector('.copy-btn-vanilla')) return;
      
      pre.style.position = 'relative';
      const btn = document.createElement('button');
      btn.className = 'copy-btn-vanilla absolute top-3 right-3 p-2 rounded-lg bg-white/80 backdrop-blur-sm border border-black/5 text-neutral/40 hover:text-primary transition-all z-10 cursor-pointer flex items-center justify-center gap-1 opacity-0 group-hover:opacity-100';
      btn.innerHTML = '<svg width="14" height="14" viewBox="0 0 256 256"><path fill="currentColor" d="M216,40H88a16,16,0,0,0-16,16V72H56A16,16,0,0,0,40,88V216a16,16,0,0,0,16,16H184a16,16,0,0,0,16-16V200h16a16,16,0,0,0,16-16V56A16,16,0,0,0,216,40ZM184,216H56V88H184V216Zm32-32H200V88a16,16,0,0,0-16-16H88V56H216V184Z"></path></svg>';
      btn.title = 'Copiar código';
      
      // Adicionar classe group ao pre para o hover
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

  const handleDocSelect = (doc) => {
    setCurrentDoc(doc);
    loadDoc(doc.path);
    setIsMobileSidebarOpen(false);
  };

  const currentIndex = sectionDocs.findIndex(d => d.path === currentDoc?.path);
  const prevDoc = currentIndex > 0 ? sectionDocs[currentIndex - 1] : null;
  const nextDoc = currentIndex < sectionDocs.length - 1 ? sectionDocs[currentIndex + 1] : null;

  return (
    <div data-z-id="docviewer-page" className="page-doc-viewer flex w-full bg-[#ECEEEE] transition-all duration-500 relative">
      <ReadingProgressBar />
      
      {/* MOBILE SIDEBAR OVERLAY */}
      <AnimatePresence>
        {isMobileSidebarOpen && (
          <>
            <motion.div 
              initial={{ opacity: 0 }}
              animate={{ opacity: 1 }}
              exit={{ opacity: 0 }}
              onClick={() => setIsMobileSidebarOpen(false)}
              className="fixed inset-0 bg-black/20 backdrop-blur-sm z-[60] lg:hidden"
            />
            <motion.div
              initial={{ x: -280 }}
              animate={{ x: 0 }}
              exit={{ x: -280 }}
              transition={{ type: "spring", damping: 25, stiffness: 200 }}
              className="fixed left-0 top-0 bottom-0 w-[280px] bg-white z-[70] lg:hidden shadow-2xl"
            >
              <Sidebar 
                sections={DOCS_STRUCTURE[section] || []} 
                currentDoc={currentDoc}
                onDocSelect={handleDocSelect}
                isMobile={true}
              />
            </motion.div>
          </>
        )}
      </AnimatePresence>

      {/* DESKTOP SIDEBAR */}
      {!isZenMode && (
        <aside 
          data-z-id="docviewer-sidebar-wrapper" 
          className="doc-sidebar shrink-0 hidden lg:block"
        >
          <Sidebar 
            sections={DOCS_STRUCTURE[section] || []} 
            currentDoc={currentDoc}
            onDocSelect={handleDocSelect}
          />
        </aside>
      )}
      
      <main data-z-id="docviewer-main-content" className={`doc-content flex-1 px-6 py-10 md:px-16 md:py-16 mx-auto min-h-[calc(100vh-64px)] relative transition-all duration-500 w-full ${isZenMode ? 'max-w-3xl' : 'max-w-4xl'}`}>
        {/* Floating Mobile Toggle */}
        <button
          onClick={() => setIsMobileSidebarOpen(true)}
          className="lg:hidden fixed bottom-8 right-8 z-50 bg-[#111111] text-white p-4 rounded-full shadow-2xl flex items-center gap-2 hover:scale-105 active:scale-95 transition-all"
        >
          <svg xmlns="http://www.w3.org/2000/svg" width="20" height="20" fill="currentColor" viewBox="0 0 256 256"><path d="M224,128a8,8,0,0,1-8,8H40a8,8,0,0,1,0-16H216A8,8,0,0,1,224,128ZM40,72H216a8,8,0,0,0,0-16H40a8,8,0,0,0,0-16ZM216,184H40a8,8,0,0,0,0,16H216a8,8,0,0,0,0-16Z"></path></svg>
          <span className="text-sm font-bold pr-2">Tópicos</span>
        </button>

        <div data-z-id="docviewer-breadcrumbs-wrapper" className="doc-breadcrumbs mb-8 flex flex-col sm:flex-row items-start sm:items-center justify-between gap-4">
          <Breadcrumbs 
            section={section} 
            category={currentDoc?.category} 
            label={currentDoc?.label} 
          />
          
          <motion.button 
            whileHover={{ scale: 1.05 }}
            whileTap={{ scale: 0.95 }}
            onClick={() => setIsZenMode(!isZenMode)}
            className={`px-4 py-2 rounded-full border transition-all flex items-center gap-2 text-[10px] font-bold uppercase tracking-widest ${isZenMode ? 'bg-primary text-white border-primary shadow-lg shadow-primary/20' : 'bg-white border-black/5 text-neutral/40 hover:text-primary hover:border-primary/20'}`}
          >
            {isZenMode ? "Sair do Modo Zen" : "Modo Zen"}
          </motion.button>
        </div>

        {loading ? (
          <div data-z-id="docviewer-loading-skeleton" className="doc-loading flex flex-col gap-6 animate-pulse">
            <div data-z-id="docviewer-skeleton-1" className="h-10 bg-black/5 rounded-xl w-3/4" />
            <div data-z-id="docviewer-skeleton-2" className="h-4 bg-black/5 rounded-lg w-full" />
            <div data-z-id="docviewer-skeleton-3" className="h-4 bg-black/5 rounded-lg w-5/6" />
            <div data-z-id="docviewer-skeleton-4" className="h-4 bg-black/5 rounded-lg w-full" />
            <div data-z-id="docviewer-skeleton-5" className="h-64 bg-black/5 rounded-2xl w-full mt-8" />
          </div>
        ) : (
          <AnimatePresence mode="wait">
            <motion.article 
              key={currentDoc?.path}
              initial={{ opacity: 0, y: 10 }}
              animate={{ opacity: 1, y: 0 }}
              exit={{ opacity: 0, y: -10 }}
              transition={{ duration: 0.3, ease: "easeOut" }}
              data-z-id="docviewer-article"
              ref={articleRef}
              className="doc-article prose prose-slate prose-lg max-w-none 
                         prose-headings:font-display prose-headings:text-neutral
                         prose-pre:bg-[#FAFAFA] prose-pre:border prose-pre:border-black/5 prose-pre:shadow-sm
                         prose-a:text-primary hover:prose-a:text-primary-hover transition-colors
                         prose-img:rounded-3xl prose-blockquote:rounded-r-xl w-full overflow-x-hidden"
              dangerouslySetInnerHTML={{ __html: content }} 
            />
            
            <motion.div 
              key={`nav-${currentDoc?.path}`}
              initial={{ opacity: 0 }}
              animate={{ opacity: 1 }}
              transition={{ delay: 0.2, duration: 0.3 }}
              data-z-id="docviewer-pagination-wrapper" 
              className="doc-pagination mt-20 pt-8 border-t border-black/10"
            >
              <Pagination 
                prev={prevDoc} 
                next={nextDoc} 
                onNavigate={handleDocSelect} 
              />
            </motion.div>
          </AnimatePresence>
        )}
      </main>

      {/* TOC Sidebar - Hidden on Tablet/Mobile */}
      {!isZenMode && (
        <aside 
          data-z-id="docviewer-toc-wrapper" 
          className="doc-toc shrink-0 hidden xl:block w-64"
        >
          <TableOfContents content={content} />
        </aside>
      )}
    </div>
  );
};

export default DocViewer;