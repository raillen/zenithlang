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
10. comandos Tauri para ler/salvar texto e carregar snapshot local.

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

1. substituir o fallback por dados reais do projeto aberto;
2. conectar save de cena JSON;
3. adicionar viewport Three.js real;
4. conectar preview/runtime Raylib;
5. adicionar atalhos, command palette e diagnostics.
