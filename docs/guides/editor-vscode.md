# Editor VSCode para Zenith

Este guia cobre o uso local da extensao beta `tools/vscode-zenith`.

## O que funciona neste corte

- Diagnostics em tempo real para arquivos `.zt` abertos.
- Autocomplete de palavras-chave, tipos, snippets basicos, simbolos do arquivo aberto, variaveis locais e parametros.
- Autocomplete em `import ...`:
  - sugere módulos `std.*`;
  - sugere namespaces de módulos `.zt` indexados no workspace, como `app.util` ou `borealis.game`.
- Autocomplete contextual depois de `.`:
  - `list<T>.` sugere `get(index)`.
  - `map<K,V>.` sugere `get(key)`.
  - `alias.` sugere membros publicos do namespace importado por `import ... as alias`.
  - `core.Error.` sugere `code`, `message` e `context`.
  - `Struct.` em variavel tipada sugere campos declarados da struct.
- Hover de simbolos declarados no arquivo atual.
- Hover de simbolos declarados em arquivos `.zt` indexados no workspace.
- Go-to-definition para simbolos declarados em arquivos `.zt` indexados no workspace.
- Formatacao pelo formatter oficial usado pelo compilador.
- Comandos rapidos no editor:
  - `Zenith: Check Project`
  - `Zenith: Build Project`
  - `Zenith: Run Project`

## Passo 1: compilar o LSP

Na raiz do repositorio:

```powershell
python tools/build_lsp.py
```

Resultado esperado:

```text
SUCCESS
```

Isso gera `zt-lsp.exe` no Windows ou `zt-lsp` em Linux/macOS.

## Passo 2: abrir a extensao em modo desenvolvimento

1. Abra a pasta `tools/vscode-zenith` no VSCode.
2. Pressione `F5`.
3. Uma nova janela do VSCode sera aberta.
4. Nessa nova janela, abra um projeto Zenith.
5. Abra um arquivo `.zt`.

## Passo 3: ativar format-on-save

No VSCode, habilite:

```json
{
  "editor.formatOnSave": true,
  "[zenith]": {
    "editor.defaultFormatter": "zenith-lang.zenith-vscode"
  }
}
```

Se preferir nao formatar ao salvar, mantenha `editor.formatOnSave` como `false`.

## Configuracoes uteis

```json
{
  "zenith.lsp.path": "C:/caminho/para/zt-lsp.exe",
  "zenith.cli.path": "C:/caminho/para/zt.exe"
}
```

Use essas configuracoes quando o VSCode nao encontrar os binarios automaticamente.

## Como validar rapido

Na raiz do repositorio:

```powershell
python tests/lsp/test_lsp_smoke.py
node --check tools/vscode-zenith/extension.js
```

Resultado esperado:

```text
lsp smoke ok
```

## Limites conhecidos

- O indice do workspace e textual/in-memory: a extensao indexa arquivos `.zt` do workspace e atualiza em create/change/delete.
- Hover, completion e go-to-definition cross-file usam `namespace`, `import ... as ...` e `public` para reduzir sugestoes fora de contexto.
- A resolucao ainda nao substitui o typechecker completo do projeto. Para erros definitivos, use `zt check`.
- O autocomplete de membro sugere apenas recursos aceitos pelo compilador atual. Por isso `list<T>.append(...)` e `list<T>.prepend(...)` nao aparecem enquanto nao forem suportados semanticamente.
- O LSP usa sincronizacao full-document.
- A extensao beta ainda nao esta publicada no Marketplace.
- Os comandos `check/build/run` usam o terminal integrado do VSCode.
