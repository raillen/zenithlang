import React, { useState, useEffect, useRef } from 'react';
import { motion, AnimatePresence } from 'framer-motion';
import { findDocByFile, getAllDocs } from '../data/docs';

const ALL_DOCS = getAllDocs();

const SearchModal = ({ isOpen, onClose, onSelect }) => {
  const [query, setQuery] = useState('');
  const [debouncedQuery, setDebouncedQuery] = useState('');
  const [results, setResults] = useState([]);
  const [selectedIndex, setSelectedIndex] = useState(0);
  const inputRef = useRef(null);

  useEffect(() => {
    const timer = setTimeout(() => setDebouncedQuery(query), 120);
    return () => clearTimeout(timer);
  }, [query]);

  useEffect(() => {
    if (isOpen) {
      setQuery('');
      setDebouncedQuery('');
      setSelectedIndex(0);
      setTimeout(() => inputRef.current?.focus(), 100);
    }
  }, [isOpen]);

  useEffect(() => {
    if (debouncedQuery.trim() === '') {
      setResults([]);
      return;
    }

    const filtered = ALL_DOCS.filter(
      (doc) =>
        doc.label.toLowerCase().includes(debouncedQuery.toLowerCase()) ||
        doc.category.toLowerCase().includes(debouncedQuery.toLowerCase()) ||
        (doc.summary || '').toLowerCase().includes(debouncedQuery.toLowerCase()),
    ).slice(0, 8);

    setResults(filtered);
    setSelectedIndex(0);
  }, [debouncedQuery]);

  const handleKeyDown = (e) => {
    if (e.key === 'ArrowDown') {
      setSelectedIndex((prev) => (prev + 1) % currentResults.length);
    } else if (e.key === 'ArrowUp') {
      setSelectedIndex((prev) => (prev - 1 + currentResults.length) % currentResults.length);
    } else if (e.key === 'Enter' && currentResults[selectedIndex]) {
      handleSelect(currentResults[selectedIndex]);
    } else if (e.key === 'Escape') {
      onClose();
    }
  };

  const handleSelect = (doc) => {
    if (doc.isAction) {
      doc.action();
    } else {
      onSelect(doc);
    }
    onClose();
  };

  const quickActions = [
    {
      label: 'Voltar para Home',
      section: 'HOME',
      category: 'Navegação',
      path: 'home',
      isAction: true,
      action: () => onSelect({ section: 'home' }),
    },
    findDocByFile('language/current.md'),
    findDocByFile('compiler/ascension-zenith.md'),
    findDocByFile('versions/0.3.x-ascension.md'),
    findDocByFile('roadmap/implementations.md'),
  ].filter(Boolean);

  const currentResults = debouncedQuery.trim() === '' ? quickActions : results;

  return (
    <AnimatePresence>
      {isOpen && (
        <div data-z-id="search-modal-container" className="search-modal-overlay fixed inset-0 z-[100] flex items-start justify-center px-4 pt-[15vh] sm:px-6">
          <motion.div
            data-z-id="search-modal-backdrop"
            initial={{ opacity: 0 }}
            animate={{ opacity: 1 }}
            exit={{ opacity: 0 }}
            onClick={onClose}
            className="search-modal-backdrop fixed inset-0 bg-black/20 backdrop-blur-sm"
          />

          <motion.div
            data-z-id="search-modal-content"
            initial={{ opacity: 0, scale: 0.98, y: -10 }}
            animate={{ opacity: 1, scale: 1, y: 0 }}
            exit={{ opacity: 0, scale: 0.98, y: -10 }}
            className="search-modal-content relative w-full max-w-2xl overflow-hidden rounded-2xl border border-white/20 bg-white/95 shadow-2xl backdrop-blur-xl"
            role="dialog"
            aria-modal="true"
            aria-label="Buscar documentação"
            style={{ willChange: 'transform' }}
          >
            <div data-z-id="search-modal-input-wrapper" className="search-modal-input-wrapper flex items-center gap-4 border-b border-black/5 p-6">
              <svg data-z-id="search-modal-input-icon" xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={2} stroke="currentColor" className="h-6 w-6 text-primary" aria-hidden="true">
                <path strokeLinecap="round" strokeLinejoin="round" d="M21 21l-5.197-5.197m0 0A7.5 7.5 0 105.196 5.196a7.5 7.5 0 0010.607 10.607z" />
              </svg>
              <input
                data-z-id="search-modal-input"
                ref={inputRef}
                type="text"
                placeholder="O que você está procurando?"
                className="search-modal-input flex-1 border-none bg-transparent text-xl font-medium text-neutral outline-none placeholder:text-neutral/20"
                value={query}
                onChange={(e) => setQuery(e.target.value)}
                onKeyDown={handleKeyDown}
              />
              <div data-z-id="search-modal-hint-esc" className="search-modal-hint rounded border border-black/10 px-2 py-1 text-[10px] font-bold text-neutral/40">ESC</div>
            </div>

            <div data-z-id="search-modal-results" className="search-modal-results max-h-[60vh] overflow-y-auto p-3" role="listbox" style={{ contain: 'content' }}>
              {debouncedQuery.trim() === '' && (
                <div className="px-3 py-2 text-[10px] font-bold uppercase tracking-widest text-neutral/30">Sugestões Rápidas</div>
              )}

              {currentResults.length > 0 ? (
                <div data-z-id="search-modal-results-list" className="flex flex-col gap-1" role="list">
                  <AnimatePresence mode="popLayout">
                    {currentResults.map((doc, index) => (
                      <motion.button
                        key={doc.path + doc.label}
                        initial={{ opacity: 0, y: 5 }}
                        animate={{ opacity: 1, y: 0 }}
                        exit={{ opacity: 0, y: -5 }}
                        transition={{ duration: 0.2 }}
                        data-z-id={`search-result-item-${index}`}
                        onClick={() => handleSelect(doc)}
                        onMouseEnter={() => setSelectedIndex(index)}
                        role="option"
                        aria-selected={selectedIndex === index}
                        className={`search-result-item flex w-full items-center justify-between rounded-xl p-4 text-left transition-all duration-200 ${
                          selectedIndex === index ? 'bg-primary text-white shadow-lg shadow-primary/20 -translate-y-0.5' : 'text-neutral hover:bg-black/10'
                        }`}
                      >
                        <div data-z-id="search-result-item-content" className="flex flex-col gap-0.5">
                          <span data-z-id="search-result-item-label" className={`search-result-label text-base font-semibold ${selectedIndex === index ? 'text-white' : 'text-neutral'}`}>
                            {doc.label}
                          </span>
                          <span data-z-id="search-result-item-meta" className={`search-result-meta text-xs ${selectedIndex === index ? 'text-white/70' : 'text-neutral/40'}`}>
                            {(doc.sectionLabel || doc.section).toUpperCase()} - {doc.category}
                          </span>
                        </div>
                        <svg data-z-id="search-result-item-icon" xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={2.5} stroke="currentColor" className={`h-4 w-4 transition-transform ${selectedIndex === index ? 'translate-x-1 text-white' : 'text-neutral/20'}`} aria-hidden="true">
                          <path strokeLinecap="round" strokeLinejoin="round" d="M8.25 4.5l7.5 7.5-7.5 7.5" />
                        </svg>
                      </motion.button>
                    ))}
                  </AnimatePresence>
                </div>
              ) : (
                <div data-z-id="search-modal-no-results" className="p-10 text-center text-neutral/40">
                  <p data-z-id="search-modal-no-results-text" className="text-base font-serif italic">Nenhum resultado encontrado para "{query}"</p>
                </div>
              )}
            </div>

            <div data-z-id="search-modal-footer" className="search-modal-footer flex items-center justify-between border-t border-black/5 bg-neutral/5 p-4 text-sm text-neutral/50">
              <div data-z-id="search-modal-keyboard-shortcuts" className="flex gap-6">
                <span data-z-id="search-modal-shortcut-nav" className="flex items-center gap-2">
                  <kbd data-z-id="search-modal-kbd-arrows" className="rounded border border-black/10 bg-white px-2 py-1 text-xs font-medium">UP/DOWN</kbd> Navegar
                </span>
                <span data-z-id="search-modal-shortcut-select" className="flex items-center gap-2">
                  <kbd data-z-id="search-modal-kbd-enter" className="rounded border border-black/10 bg-white px-2 py-1 text-xs font-medium">ENTER</kbd> Selecionar
                </span>
              </div>
              <div data-z-id="search-modal-footer-branding" className="flex items-center gap-1">
                Busca por <span data-z-id="search-modal-footer-brand-name" className="font-medium text-primary">Zenith Hub</span>
              </div>
            </div>
          </motion.div>
        </div>
      )}
    </AnimatePresence>
  );
};

export default SearchModal;