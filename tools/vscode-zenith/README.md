# Zenith VSCode Beta

Extensao beta para arquivos `.zt`.

## Recursos

- Diagnostics em tempo real via Compass LSP.
- Hover de simbolos do arquivo atual.
- Go-to-definition para simbolos declarados no arquivo atual.
- Formatacao pelo formatter oficial do compilador.
- Comandos rapidos: `Zenith: Check Project`, `Zenith: Build Project`, `Zenith: Run Project`.

## Uso local

1. Compile o LSP:

```powershell
python tools/build_lsp.py
```

2. Abra `tools/vscode-zenith` no VSCode como pasta de extensao.
3. Pressione `F5` para abrir uma Extension Development Host.
4. Abra um arquivo `.zt`.

## Configuracao

- `zenith.lsp.path`: caminho para `zt-lsp` ou `zt-lsp.exe`.
- `zenith.cli.path`: caminho para `zt` ou `zt.exe`.

## Limites da beta

- Hover e definition v1 focam simbolos declarados no documento aberto.
- Diagnostics usam sincronizacao full-document.
- Comandos `check/build/run` executam no terminal integrado.
