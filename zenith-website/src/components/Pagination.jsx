import React from 'react';

const Pagination = ({ prev, next, onNavigate }) => {
  if (!prev && !next) return null;

  return (
    <div data-z-id="pagination-container" className="mt-16 pt-8 border-t border-black/5 flex justify-between items-center gap-4">
      {prev ? (
        <button
          data-z-id="pagination-prev-btn"
          onClick={() => onNavigate(prev)}
          className="flex-1 group p-4 rounded-2xl border border-black/5 hover:border-primary/20 hover:bg-primary/5 transition-all text-left"
        >
          <div data-z-id="pagination-prev-label" className="text-[10px] font-bold text-neutral/30 uppercase tracking-widest mb-1">Anterior</div>
          <div data-z-id="pagination-prev-content" className="flex items-center gap-2 font-bold text-neutral group-hover:text-primary transition-colors">
            <svg data-z-id="pagination-prev-icon" xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={2.5} stroke="currentColor" className="w-4 h-4">
              <path strokeLinecap="round" strokeLinejoin="round" d="M10.5 19.5L3 12m0 0l7.5-7.5M3 12h18" />
            </svg>
            {prev.label}
          </div>
        </button>
      ) : <div data-z-id="pagination-prev-placeholder" className="flex-1" />}

      {next ? (
        <button
          data-z-id="pagination-next-btn"
          onClick={() => onNavigate(next)}
          className="flex-1 group p-4 rounded-2xl border border-black/5 hover:border-primary/20 hover:bg-primary/5 transition-all text-right"
        >
          <div data-z-id="pagination-next-label" className="text-[10px] font-bold text-neutral/30 uppercase tracking-widest mb-1">Próximo</div>
          <div data-z-id="pagination-next-content" className="flex items-center justify-end gap-2 font-bold text-neutral group-hover:text-primary transition-colors">
            {next.label}
            <svg data-z-id="pagination-next-icon" xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={2.5} stroke="currentColor" className="w-4 h-4">
              <path strokeLinecap="round" strokeLinejoin="round" d="M13.5 4.5L21 12m0 0l-7.5 7.5M21 12H3" />
            </svg>
          </div>
        </button>
      ) : <div data-z-id="pagination-next-placeholder" className="flex-1" />}
    </div>
  );
};

export default Pagination;