import React, { useState, useRef, useEffect } from 'react';
import { motion, AnimatePresence } from 'framer-motion';
import { MagnifyingGlass, CaretDown, Code, Hash, X } from '@phosphor-icons/react';

const Breadcrumbs = ({ section, category, label, symbols = [], onSymbolSelect }) => {
  const [isOpen, setIsOpen] = useState(false);
  const [search, setSearch] = useState('');
  const dropdownRef = useRef(null);
  const inputRef = useRef(null);

  const sectionLabels = {
    home: 'Início',
    handbook: 'Documentação',
    api: 'API',
    pedagogy: 'Aprender',
    architecture: 'Arquitetura',
    roadmap: 'Roadmap',
    changelog: 'Changelog',
    about: 'Sobre'
  };

  useEffect(() => {
    const handleClickOutside = (event) => {
      if (dropdownRef.current && !dropdownRef.current.contains(event.target)) {
        setIsOpen(false);
      }
    };
    document.addEventListener('mousedown', handleClickOutside);
    return () => document.removeEventListener('mousedown', handleClickOutside);
  }, []);

  useEffect(() => {
    if (isOpen) {
      setSearch('');
      setTimeout(() => inputRef.current?.focus(), 100);
    }
  }, [isOpen]);

  const filteredSymbols = symbols.filter(s => 
    s.label.toLowerCase().includes(search.toLowerCase())
  );

  return (
    <nav data-z-id="breadcrumbs-container" className="doc-breadcrumb flex items-center gap-3 text-sm text-neutral/50" aria-label="Breadcrumb">
      <ol data-z-id="breadcrumbs-list" className="flex items-center gap-3 list-none">
        <li data-z-id="breadcrumbs-item-root">
          <span className="breadcrumb-item hover:text-primary cursor-pointer transition-colors px-1 font-medium">Zenith</span>
        </li>
        <li className="text-neutral/20" aria-hidden="true">/</li>
        <li data-z-id="breadcrumbs-item-section">
          <span className="breadcrumb-item hover:text-primary cursor-pointer transition-colors px-1 font-medium">{sectionLabels[section] || section}</span>
        </li>
        {category && (
          <>
            <li className="text-neutral/20" aria-hidden="true">/</li>
            <li data-z-id="breadcrumbs-item-category">
              <span className="breadcrumb-category px-1 font-medium">{category}</span>
            </li>
          </>
        )}
        <li className="text-neutral/20" aria-hidden="true">/</li>
        
        {/* INTERACTIVE SYMBOL NAVIGATOR */}
        <li data-z-id="breadcrumbs-item-current" className="relative" ref={dropdownRef}>
          <button 
            data-z-id="breadcrumb-jump-trigger"
            onClick={() => setIsOpen(!isOpen)}
            className={`flex items-center gap-2 px-3 py-1 rounded-full font-medium transition-all outline-none focus-visible:ring-2 focus-visible:ring-primary ${
              isOpen ? 'bg-primary text-white shadow-lg shadow-primary/20 scale-105' : 'bg-primary/10 text-primary hover:bg-primary/20'
            }`}
          >
            {label}
            <CaretDown size={12} weight="bold" className={`transition-transform duration-300 ${isOpen ? 'rotate-180' : ''}`} />
          </button>

          <AnimatePresence mode="wait">
            {isOpen && (
              <motion.div
                data-z-id="breadcrumb-jump-dropdown"
                initial={{ opacity: 0, y: 10, scale: 0.95 }}
                animate={{ opacity: 1, y: 0, scale: 1 }}
                exit={{ opacity: 0, y: 10, scale: 0.95 }}
                className="absolute left-0 lg:left-0 top-full mt-2 w-72 bg-white rounded-2xl shadow-2xl border border-black/5 z-[100] overflow-hidden"
              >
                <div className="p-3 border-b border-black/5 flex items-center gap-2">
                  <MagnifyingGlass size={16} className="text-neutral/30" />
                  <input
                    ref={inputRef}
                    type="text"
                    placeholder="Pular para Símbolo..."
                    className="flex-1 bg-transparent border-none outline-none text-sm text-neutral placeholder:text-neutral/20"
                    value={search}
                    onChange={(e) => setSearch(e.target.value)}
                  />
                  {search && (
                    <button onClick={() => setSearch('')} className="text-neutral/30 hover:text-neutral">
                      <X size={14} />
                    </button>
                  ) }
                </div>

                <div className="max-h-64 overflow-y-auto p-2 scrollbar-none">
                  {filteredSymbols.length > 0 ? (
                    <div className="flex flex-col gap-1">
                      {filteredSymbols.map((s) => (
                        <button
                          key={s.id}
                          onClick={() => {
                            onSymbolSelect(s.id);
                            setIsOpen(false);
                          }}
                          className="w-full text-left p-3 rounded-xl hover:bg-black/5 transition-colors flex items-center gap-3 group"
                        >
                          <div className={`p-1.5 rounded-lg ${s.type === 'function' ? 'bg-indigo-50 text-indigo-500' : 'bg-amber-50 text-amber-500'}`}>
                            {s.type === 'function' ? <Code size={14} weight="bold" /> : <Hash size={14} weight="bold" />}
                          </div>
                          <div className="flex flex-col min-w-0">
                            <span className="text-sm font-semibold text-neutral truncate group-hover:text-primary transition-colors">
                              {s.label}
                            </span>
                            <span className="text-[10px] font-bold text-neutral/30 uppercase tracking-widest">
                              {s.type === 'function' ? 'Módulo / API' : 'Seção'}
                            </span>
                          </div>
                        </button>
                      ))}
                    </div>
                  ) : (
                    <div className="p-8 text-center text-xs text-neutral/30 italic">
                      Nenhum símbolo encontrado
                    </div>
                  )}
                </div>
              </motion.div>
            )}
          </AnimatePresence>
        </li>
      </ol>
    </nav>
  );
};

export default Breadcrumbs;