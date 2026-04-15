import React from 'react';

const stateLabels = {
  pt: { archived: 'Arquivado', current: 'Atual', deprecated: 'Obsoleto', experimental: 'Experimental', stable: 'Estável', vision: 'Visão' },
  en: { archived: 'Archived', current: 'Current', deprecated: 'Deprecated', experimental: 'Experimental', stable: 'Stable', vision: 'Vision' },
  ja: { archived: 'アーカイブ済み', current: '最新', deprecated: '非推奨', experimental: '実験的', stable: '安定', vision: 'ビジョン' },
  es: { archived: 'Archivado', current: 'Actual', deprecated: 'Obsoleto', experimental: 'Experimental', stable: 'Estable', vision: 'Visión' }
};

const DocPageHeader = ({ doc, lang }) => {
  if (!doc) return null;

  return (
    <header className="mb-12">
      <div className="mb-4 flex flex-wrap items-center gap-x-3 gap-y-2">
        <span className={`text-[9px] font-black uppercase tracking-widest ${
          doc.state === 'current' ? 'text-primary' : 
          doc.state === 'stable' ? 'text-emerald-600' :
          'text-neutral/40'
        }`}>
          {stateLabels[lang]?.[doc.state] || doc.state || 'Doc'}
        </span>
        <span className="h-3 w-px bg-black/10" />
        <span className="text-[9px] font-bold uppercase tracking-widest text-neutral/30">
          {doc.category}
        </span>
        <span className="h-3 w-px bg-black/10" />
        <span className="text-[9px] font-bold uppercase tracking-widest text-neutral/30">
          {doc.sectionLabel}
        </span>
      </div>
      <h1 className="text-4xl font-black tracking-tight text-gray-900 md:text-6xl">
        {doc.label}
      </h1>
      {doc.summary && (
        <p className="mt-6 max-w-[60ch] text-xl leading-relaxed text-gray-500/80 font-medium">
          {doc.summary}
        </p>
      )}
    </header>
  );
};

export default DocPageHeader;
