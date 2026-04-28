# Borealis Studio

Novo editor desktop do Borealis em `Tauri + React + TypeScript`.

## Direcao de produto

- Visual: shell clara e produtiva, inspirada no fluxo do Unity e traduzida para o contrato do Borealis.
- Usabilidade: fluxo de editor de jogo com toolbar lateral, Hierarchy, Scene/Game, Inspector, Project, Console e Animation.
- Scripts: editor integrado para abrir e editar scripts associados aos objetos.
- Icones: `lucide-react`.
- Runtime: Tauri fica responsavel por ponte Rust, arquivos locais e preview.

## Primeiro corte

Este scaffold ja entrega:

1. shell Tauri 2;
2. React + Vite + TypeScript;
3. shell Unity-like com header global, tool rail e status bar;
4. layout dockavel com paineis redimensionaveis e persistencia local;
5. Hierarchy real com parent/child e reparent por drag-and-drop;
6. Scene/Game no centro, com Scene View 3D visual e gizmos;
7. Inspector com fluxo de objeto/cena e transform 3D;
8. Project + Console + Animation em dock inferior;
9. editor de codigo integrado para scripts Zenith;
10. fallback browser com dados mockados;
11. comandos Tauri para ler/salvar texto e carregar snapshot local;
12. save real de cena JSON;
13. Play/Pause/Stop conectados ao preview sidecar por JSONL stdio;
14. tela inicial com abrir projeto, novo projeto, templates e links locais de documentacao.

## Planejamento ativo

- Roadmap: `docs/internal/planning/borealis-studio-roadmap-v1.md`.
- Checklist: `docs/internal/planning/borealis-studio-checklist-v1.md`.

## Preview runtime

Os controles de Play usam o backend Tauri para iniciar o preview real como processo separado.

Fluxo atual:

1. o Studio serializa a cena aberta;
2. grava uma copia temporaria em `.ztc-tmp/borealis-studio/preview.scene.json`;
3. inicia o runner de preview;
4. envia `open_project`, `open_scene` e `enter_play_mode`;
5. mostra status e eventos no Console do editor.

Ordem de runner em uma distribuicao independente:

1. `runtime/sdk/bin/borealis-preview.exe`;
2. `runtime/sdk/preview/borealis-preview.exe`;
3. `runtime/sdk/bin/zt.exe run runtime/sdk/preview/zenith.ztproj`.

No desenvolvimento dentro deste repositorio, use `BOREALIS_SDK_ROOT` apontando
para um SDK local com `bin/zt.exe` e `preview/zenith.ztproj`.

O antigo prototipo de preview nao faz parte da arvore atual.

O Studio tambem aceita `BOREALIS_SDK_ROOT` para apontar para um SDK local.
Isso permite rodar o app fora do repositorio da linguagem sem copiar o codigo fonte inteiro.

Para montar esse SDK no workspace local:

```powershell
cd tools/borealis-studio
npm run sdk:assemble
```

O comando copia `zt.exe`, `runtime`, `stdlib`, `packages/borealis`, manifesto,
templates e o projeto `preview/zenith.ztproj` para `runtime/sdk`.

Layout recomendado para release:

```text
borealis-studio/
  Borealis Studio.exe
  runtime/
    sdk/
      bin/
        zt.exe
        borealis-preview.exe
      stdlib/
      preview/
        zenith.ztproj
      templates/
```

## Fluxo de projeto

A tela inicial e o menu `Start` viraram a entrada principal do Studio.

Fluxos atuais:

1. abrir um arquivo `zenith.ztproj`;
2. abrir uma pasta que tenha `zenith.ztproj`;
3. criar um projeto novo com template;
4. continuar pelo pacote Borealis do proprio repositorio.

O backend agora resolve tres bases:

1. `appRoot`: a pasta do Borealis Studio;
2. `sdkRoot`: `BOREALIS_SDK_ROOT`, pasta runtime SDK ou pasta `sdk`;
3. `repoRoot`: fallback de desenvolvimento quando o app roda dentro de `zenith-lang-v2`.

Em release, o Studio deve depender do SDK empacotado, nao do repositorio da linguagem.

## Manifesto de editor

O Studio carrega `borealis.editor.json` para descobrir componentes, campos, tipos de asset e acoes do editor.

Ordem de busca:

1. `BOREALIS_SDK_ROOT/borealis.editor.json`;
2. `BOREALIS_SDK_ROOT/editor/borealis.editor.json`;
3. `packages/borealis/borealis.editor.json`, quando estiver rodando dentro do repositorio;
4. `runtime/sdk/borealis.editor.json`, quando estiver empacotado.

Se nenhum manifesto existir, o frontend usa o catalogo interno como fallback.
Esse fallback e temporario: recursos novos de engine devem entrar primeiro no manifesto do package/SDK.

## Como rodar

```powershell
cd tools/borealis-studio
npm install
npm run dev
```

Para rodar como desktop:

```powershell
cd tools/borealis-studio
npm install
npm run tauri dev
```

## Proximos passos

1. adicionar dialog nativo para escolher pasta/arquivo;
2. evoluir o preview de status JSONL para imagem/frame embutido no viewport;
3. evoluir o empacotamento para anexar o SDK montado no instalador final;
4. adicionar atalhos, command palette e diagnostics;
5. ampliar testes automatizados do backend Tauri.
