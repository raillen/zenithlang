import React, { useState, useEffect } from 'react';
import { motion, AnimatePresence } from 'framer-motion';

const TableOfContents = ({ content }) => {
  const [headings, setHeadings] = useState([]);
  const [isExpanded, setIsExpanded] = useState(true);

  useEffect(() => {
    // Parsear cabeçalhos do HTML renderizado
    const parser = new DOMParser();
    const doc = parser.parseFromString(content, 'text/html');
    const h2s = Array.from(doc.querySelectorAll('h2'));
    
    const parsedHeadings = h2s.map(h => ({
      id: h.id || h.innerText.toLowerCase().replace(/\s+/g, '-'),
      text: h.innerText,
      level: 2
    }));

    setHeadings(parsedHeadings);
  }, [content]);

  if (headings.length === 0) return null;

  return (
    <div data-z-id="toc-container" className="hidden xl:block fixed right-12 top-24 w-64 z-40">
      <div data-z-id="toc-wrapper" className="relative border-l border-black/5 pl-6 py-2">
        <button 
          data-z-id="toc-toggle-btn"
          onClick={() => setIsExpanded(!isExpanded)}
          className="flex items-center gap-2 text-[10px] font-bold uppercase tracking-widest text-neutral/30 hover:text-primary transition-colors mb-4 focus:outline-none"
        >
          <span data-z-id="toc-toggle-label">{isExpanded ? 'Recolher' : 'Sumário'}</span>
          <svg 
            data-z-id="toc-toggle-icon"
            xmlns="http://www.w3.org/2000/svg" 
            viewBox="0 0 20 20" 
            fill="currentColor" 
            className={`w-3 h-3 transition-transform ${isExpanded ? 'rotate-0' : 'rotate-180'}`}
          >
            <path strokeLinecap="round" strokeLinejoin="round" d="M3 10a.75.75 0 01.75-.75h10.5a.75.75 0 010 1.5H3.75A.75.75 0 013 10z" clipRule="evenodd" />
          </svg>
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
              <ul data-z-id="toc-list" className="flex flex-col gap-3">
                {headings.map((heading) => (
                  <li data-z-id={`toc-item-${heading.id}`} key={heading.id}>
                    <a
                      data-z-id="toc-link"
                      href={`#${heading.id}`}
                      className="block text-xs font-semibold text-neutral/40 hover:text-primary transition-colors leading-relaxed"
                    >
                      {heading.text}
                    </a>
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