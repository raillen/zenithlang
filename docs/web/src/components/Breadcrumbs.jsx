import React from 'react';

const Breadcrumbs = ({ section, category, label }) => {
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

  return (
    <nav data-z-id="breadcrumbs-container" className="doc-breadcrumb flex items-center gap-3 text-sm text-neutral/50" aria-label="Breadcrumb">
      <ol data-z-id="breadcrumbs-list" className="flex items-center gap-3 list-none">
        <li data-z-id="breadcrumbs-item-root">
          <span data-z-id="breadcrumbs-link-root" className="breadcrumb-item hover:text-primary cursor-pointer transition-colors px-1 font-medium">Zenith</span>
        </li>
        <li data-z-id="breadcrumbs-separator-1" className="text-neutral/20" aria-hidden="true">/</li>
        <li data-z-id="breadcrumbs-item-section">
          <span data-z-id="breadcrumbs-link-section" className="breadcrumb-item hover:text-primary cursor-pointer transition-colors px-1 font-medium">{sectionLabels[section] || section}</span>
        </li>
        {category && (
          <>
            <li data-z-id="breadcrumbs-separator-2" className="text-neutral/20" aria-hidden="true">/</li>
            <li data-z-id="breadcrumbs-item-category">
              <span data-z-id="breadcrumbs-link-category" className="breadcrumb-category px-1 font-medium">{category}</span>
            </li>
          </>
        )}
        <li data-z-id="breadcrumbs-separator-3" className="text-neutral/20" aria-hidden="true">/</li>
        <li data-z-id="breadcrumbs-item-current">
          <span data-z-id="breadcrumbs-label-current" className="breadcrumb-current text-primary bg-primary/10 px-3 py-1 rounded-full font-medium">{label}</span>
        </li>
      </ol>
    </nav>
  );
};

export default Breadcrumbs;