# Zenith Site

Aplicação web oficial do projeto Zenith.

## O que esta pasta contém

- Homepage com apresentação da linguagem
- Sistema de documentação completo (4 pilares)
- Visualizador de documentação markdown
- Playground interativo
- Theme Lab para experimentação visual

## Estrutura de Documentação

O site implementa 4 pilares de documentação:

1. **Narrativa** - História e filosofia da linguagem
2. **Roadmap** - Estado atual e futuro do projeto
3. **Aprender** - Trilha educativa sobre compiladores
4. **Referência** - Documentação técnica completa

## Fonte da Verdade

O conteúdo de documentação fica em `public/docs-content/`:

- `public/docs-content/pt/` - Conteúdo em português
- `public/docs-content/en/` - Conteúdo em inglês
- `public/docs-manifest.json` - Índice de toda documentação

O script `scripts/sync-docs.mjs` sincroniza conteúdo do projeto principal quando necessário.

## Comandos

```bash
npm run dev          # Servidor de desenvolvimento
npm run build        # Build de produção
npm run sync:docs    # Sincronizar documentação
npm run preview      # Preview do build
```

`npm run dev` e `npm run build` executam o sync automaticamente antes do Vite.

## Tecnologias

- React 18 + Vite
- Framer Motion (animações)
- Tailwind CSS + DaisyUI
- Marked (markdown parsing)
- Prism.js (syntax highlighting)
- React Router (navegação)

