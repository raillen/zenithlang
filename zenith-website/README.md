# Zenith Site

Aplicacao web oficial do projeto Zenith.

## O que esta pasta contem

- a homepage editorial do projeto
- o visualizador de documentacao
- a busca semantica do portal
- o playground e as paginas auxiliares do site

## Fonte da verdade

O conteudo editorial nao mora aqui.

A fonte oficial fica em `../docs`. O site consome essa biblioteca por meio de:

- `docs/site-manifest.json`
- `zenith-website/scripts/sync-docs.mjs`

O fluxo atual do portal reflete o estado self-hosted oficial da linguagem, incluindo current-core, current e os roadmaps historicos ja marcados como superados quando necessario.

## Comandos

```bash
npm run dev
npm run build
npm run sync:docs
```

`npm run dev` e `npm run build` executam o sync automaticamente antes do Vite.
