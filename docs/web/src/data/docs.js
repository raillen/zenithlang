export const DOCS_STRUCTURE = {
  handbook: [
    { title: 'Guia do Usuário', docs: [
      { label: 'Sintaxe', path: '/docs-content/handbook/syntax.md' },
      { label: 'Idiomas', path: '/docs-content/handbook/idioms.md' }
    ]},
    { title: 'Suporte', docs: [
      { label: 'Cheat Sheet', path: '/docs-content/support/cheat-sheet.md' },
      { label: 'Catálogo de Erros', path: '/docs-content/support/error-catalog.md' },
      { label: 'Cognição Visual', path: '/docs-content/support/visual-cognition.md' }
    ]}
  ],
  api: [
    { title: 'Biblioteca Padrão', docs: [
      { label: 'Core', path: '/docs-content/api/std-core.md' },
      { label: 'I/O', path: '/docs-content/api/std-io.md' },
      { label: 'Math', path: '/docs-content/api/std-math.md' }
    ]}
  ],
  pedagogy: [
    { title: 'Teoria', docs: [
      { label: '01. Introdução', path: '/docs-content/pedagogy/01-introduction.md' },
      { label: '02. Lexer Theory', path: '/docs-content/pedagogy/02-lexer-theory.md' },
      { label: '03. AST Theory', path: '/docs-content/pedagogy/03-ast-theory.md' },
      { label: '04. Parser Theory', path: '/docs-content/pedagogy/04-parser-theory.md' },
      { label: '05. Binding Theory', path: '/docs-content/pedagogy/05-binding-theory.md' },
      { label: '06. Types Theory', path: '/docs-content/pedagogy/06-types-theory.md' },
      { label: '07. CodeGen Theory', path: '/docs-content/pedagogy/07-codegen-theory.md' },
      { label: '08. Runtime Theory', path: '/docs-content/pedagogy/08-runtime-theory.md' }
    ]},
    { title: 'Implementação', docs: [
      { label: '01. Lexer Impl', path: '/docs-content/pedagogy/02-lexer-implementation.md' },
      { label: '02. AST Impl', path: '/docs-content/pedagogy/03-ast-implementation.md' },
      { label: '03. Parser Impl', path: '/docs-content/pedagogy/04-parser-implementation.md' },
      { label: '04. Binding Impl', path: '/docs-content/pedagogy/05-binding-implementation.md' },
      { label: '05. Types Impl', path: '/docs-content/pedagogy/06-types-implementation.md' },
      { label: '06. CodeGen Impl', path: '/docs-content/pedagogy/07-codegen-implementation.md' },
      { label: '07. Runtime Impl', path: '/docs-content/pedagogy/08-runtime-implementation.md' },
      { label: '10. Próximos Passos', path: '/docs-content/pedagogy/10-next-steps.md' }
    ]}
  ],
  architecture: [
    { title: 'Sistema', docs: [
      { label: 'Visão Geral', path: '/docs-content/architecture/01-overview.md' },
      { label: 'Implementação', path: '/docs-content/zenith-implementation.md' }
    ]}
  ],
  roadmap: [
    { title: 'Desenvolvimento', docs: [
      { label: 'Sprint 1', path: '/docs-content/roadmap/sprint-1.md' },
      { label: 'Sprint 2', path: '/docs-content/roadmap/sprint-2.md' },
      { label: 'Sprint 3', path: '/docs-content/roadmap/sprint-3.md' },
      { label: 'Sprint 4', path: '/docs-content/roadmap/sprint-4.md' }
    ]}
  ],
  about: [
    { title: 'Projeto', docs: [
      { label: 'Filosofia', path: '/docs-content/history/01-philosophy.md' },
      { label: 'Milestone v0.2', path: '/docs-content/history/02-v0.2-milestone.md' }
    ]}
  ],
  changelog: [
    { title: 'Histórico', docs: [
      { label: 'Changelog', path: '/docs-content/history/changelog.md' }
    ]}
  ]
};

// Helper para achatar a estrutura para busca
export const getAllDocs = () => {
  const flattened = [];
  Object.keys(DOCS_STRUCTURE).forEach(sectionKey => {
    DOCS_STRUCTURE[sectionKey].forEach(group => {
      group.docs.forEach(doc => {
        flattened.push({
          ...doc,
          section: sectionKey,
          category: group.title
        });
      });
    });
  });
  return flattened;
};
