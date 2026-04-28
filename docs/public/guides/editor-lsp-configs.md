# Editor LSP Configs

Este guia publica snippets locais para usar o `zt-lsp` fora do VSCode.

## Antes de configurar

Compile o LSP na raiz do repositorio:

```powershell
python tools/build_lsp.py
```

Resultado esperado:

```text
SUCCESS
```

No Windows, isso gera `zt-lsp.exe`.

Em Linux/macOS, isso gera `zt-lsp`.

Se o editor nao encontrar o binario, use o caminho absoluto.

## Neovim

Snippet publicado:

```text
tools/editor-configs/neovim/init.lua
```

Uso rapido:

1. Copie o conteudo para sua configuracao do Neovim.
2. Ajuste o caminho se necessario:

```lua
vim.g.zenith_lsp_path = "C:/caminho/para/zt-lsp.exe"
```

O snippet:

- registra arquivos `.zt` como `zenith`;
- procura a raiz por `zenith.ztproj` ou `.git`;
- inicia o `zt-lsp` para buffers `.zt`.

## Helix

Snippet publicado:

```text
tools/editor-configs/helix/languages.toml
```

Uso rapido:

1. Abra ou crie o arquivo `languages.toml` do Helix.
2. Adicione o bloco do snippet.
3. Troque `command = "zt-lsp"` por um caminho absoluto se necessario.

## Zed

Snippet publicado:

```text
tools/editor-configs/zed/settings.json
```

Uso rapido:

1. Abra o `settings.json` do Zed.
2. Mescle o bloco do snippet com suas configuracoes atuais.
3. Troque `"path": "zt-lsp"` por um caminho absoluto se necessario.

## Validacao rapida

Abra um arquivo `.zt`.

Depois confirme:

- diagnostics aparecem quando ha erro;
- hover e completions respondem;
- a raiz do projeto tem `zenith.ztproj`.

## Limites do corte atual

- O LSP ainda usa sincronizacao full-document.
- O binario precisa ser compilado localmente.
- A extensao VSCode segue sendo o caminho mais completo neste corte.
