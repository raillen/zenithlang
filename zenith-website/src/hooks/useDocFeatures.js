import { useEffect } from 'react';
import Prism from 'prismjs';

/**
 * Hook to manage interactive features of the document (Copy, Highlighting, Scrolling)
 */
export const useDocFeatures = (content, containerRef) => {
  useEffect(() => {
    if (!content || !containerRef.current) return;

    const container = containerRef.current;

    // 1. Syntax Highlighting
    Prism.highlightAllUnder(container);

    // 2. Add Copy Buttons to Code Blocks
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

    // 3. Handle initial hash scrolling
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
  }, [content, containerRef]);
};
