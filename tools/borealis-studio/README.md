# Borealis Studio

Novo editor desktop do Borealis em `Tauri + React + TypeScript`.

## Direcao de produto

- Visual: proximo de Codex/Xcode, escuro, denso, limpo e focado.
- Usabilidade: fluxo de editor de jogo, com Hierarchy, Scene View, Inspector, Assets e Console.
- Scripts: editor integrado para abrir e editar scripts associados aos objetos.
- Icones: `lucide-react`.
- Runtime: Tauri fica responsavel por ponte Rust, arquivos locais e preview.

## Primeiro corte

Este scaffold ja entrega:

1. shell Tauri 2;
2. React + Vite + TypeScript;
3. layout com paineis redimensionaveis;
4. Scene View 3D visual com gizmos;
5. Project/Hierarchy;
6. Inspector com transform 3D;
7. Assets + Console em dock inferior;
8. editor de codigo integrado para scripts Zenith;
9. fallback browser com dados mockados;
10. comandos Tauri para ler/salvar texto e carregar snapshot local;
11. save real de cena JSON;
12. Play/Pause/Stop conectados ao preview sidecar por JSONL stdio;
13. tela inicial com abrir projeto, novo projeto, templates e links locais de documentacao.

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

Ordem de runner no desenvolvimento dentro deste repositorio:

1. `tools/borealis-editor/preview/build/borealis-preview.exe`, se existir;
2. `zt.exe run tools/borealis-editor/preview/zenith.ztproj`.

O Studio tambem aceita `BOREALIS_SDK_ROOT` para apontar para um SDK local.
Isso permite rodar o app fora do repositorio da linguagem sem copiar o codigo fonte inteiro.

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
2. `sdkRoot`: `BOREALIS_SDK_ROOT`, `runtime/sdk` ou `sdk`;
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
2. persistir projetos recentes;
3. evoluir o preview de status JSONL para imagem/frame embutido no viewport;
4. criar pipeline de empacotamento que monta `runtime/sdk`;
5. adicionar atalhos, command palette e diagnostics;
6. ampliar testes automatizados do backend Tauri.
