import React, { useState } from 'react';
import { motion, AnimatePresence } from 'framer-motion';
import { CaretLeft, Code, Hash } from '@phosphor-icons/react';

const TableOfContents = ({ symbols = [], onSymbolSelect }) => {
  const [isExpanded, setIsExpanded] = useState(true);

  if (symbols.length === 0) return null;

  return (
    <div data-z-id="toc-container" className="hidden xl:block fixed right-12 top-24 w-64 z-40">
      <div data-z-id="toc-wrapper" className="relative border-l border-black/5 pl-6 py-2">
        <button 
          data-z-id="toc-toggle-btn"
          onClick={() => setIsExpanded(!isExpanded)}
          className="flex items-center gap-2 text-[10px] font-bold uppercase tracking-widest text-neutral/30 hover:text-primary transition-colors mb-4 focus:outline-none"
        >
          <span data-z-id="toc-toggle-label">{isExpanded ? 'Recolher Sumário' : 'Sumário'}</span>
          <CaretLeft 
            data-z-id="toc-toggle-icon"
            size={10}
            weight="bold"
            className={`transition-transform ${isExpanded ? '-rotate-90' : 'rotate-0'}`}
          />
        </button>

        <AnimatePresence>
          {isExpanded && (
            <motion.nav
              data-z-id="toc-nav"
              initial={{ opacity: 0, height: 0 }}
              animate={{ opacity: 1, height: 'auto' }}
              exit={{ opacity: 0, height: 0 }}
              className="overflow-hidden"
            >
              <ul data-z-id="toc-list" className="flex flex-col gap-1.5">
                {symbols.map((symbol) => (
                  <li data-z-id={`toc-item-${symbol.id}`} key={symbol.id} className={symbol.type === 'section' ? 'mt-3 first:mt-0' : 'ml-2'}>
                    <button
                      data-z-id="toc-link"
                      onClick={() => onSymbolSelect(symbol.id)}
                      className={`group flex items-center gap-2 text-left transition-colors ${
                        symbol.type === 'section' 
                          ? 'text-xs font-bold text-neutral hover:text-primary' 
                          : 'text-[11px] font-medium text-neutral/40 hover:text-primary'
                      }`}
                    >
                      <span className={`transition-colors ${symbol.type === 'section' ? 'text-neutral/20 group-hover:text-primary' : 'text-primary/30 group-hover:text-primary'}`}>
                        {symbol.type === 'section' ? <Hash size={12} weight="bold" /> : <Code size={11} weight="bold" />}
                      </span>
                      <span className="truncate">{symbol.label}</span>
                    </button>
                  </li>
                ))}
              </ul>
            </motion.nav>
          )}
        </AnimatePresence>
      </div>
    </div>
  );
};

export default TableOfContents;