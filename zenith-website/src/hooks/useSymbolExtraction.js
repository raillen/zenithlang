import { useState, useEffect } from 'react';

/**
 * Hook to extract symbols (headings, API rows) from HTML content
 */
export const useSymbolExtraction = (content, containerRef) => {
  const [symbols, setSymbols] = useState([]);

  useEffect(() => {
    if (!content || !containerRef.current) {
        setSymbols([]);
        return;
    }

    const newSymbols = [];
    const container = containerRef.current;
    
    // 1. Extract Headers (h2, h3)
    const headers = container.querySelectorAll('h2, h3');
    headers.forEach(header => {
      // Ensure ID exists for anchor linking
      if (!header.id) {
        header.id = header.innerText.toLowerCase()
          .replace(/[^\w\s-]/g, '')
          .replace(/\s+/g, '-');
      }
      
      newSymbols.push({
        id: header.id,
        label: header.innerText,
        type: header.tagName.toLowerCase() === 'h2' ? 'section' : 'subsection',
        top: header.offsetTop
      });
    });

    // 2. Extract API Table Members
    const tables = container.querySelectorAll('table');
    tables.forEach(table => {
      const ths = Array.from(table.querySelectorAll('th'));
      const isApiTable = ths.some(th => {
        const text = th.innerText.toLowerCase();
        return text.includes('api') || text.includes('função') || text.includes('método') || text.includes('method');
      });

      if (isApiTable) {
        const rows = table.querySelectorAll('tbody tr');
        rows.forEach((row) => {
          const firstCell = row.querySelector('td');
          if (firstCell) {
            const apiText = firstCell.innerText.trim();
            const cleanId = apiText.split('(')[0]
              .toLowerCase()
              .replace(/[^\w\s-]/g, '')
              .replace(/\s+/g, '-');
            const uniqueId = `api-${cleanId}`;
            
            row.id = uniqueId;
            row.classList.add('api-symbol-row', 'transition-colors', 'duration-500');
            
            newSymbols.push({
              id: uniqueId,
              label: apiText,
              type: 'function',
              top: row.offsetTop
            });
          }
        });
      }
    });

    setSymbols(newSymbols);
  }, [content, containerRef]);

  return symbols;
};
