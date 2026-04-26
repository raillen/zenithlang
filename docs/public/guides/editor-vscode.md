# Editor VSCode para Zenith

Este guia cobre o uso local da extensao beta `tools/vscode-zenith`.

## O que funciona neste corte

- Diagnostics em tempo real para arquivos `.zt` abertos.
- Autocomplete de palavras-chave, tipos, builtins, snippets basicos, simbolos do arquivo aberto, variaveis locais e parametros.
- Autocomplete de builtins:
  - `fmt` insere `fmt "..."`;
  - `int()`, `float()`, `int64()` e outros tipos numericos inserem conversao explicita;
  - `len()`, `to_text()`, `check()`, `todo()`, `unreachable()` e `panic()` aparecem como chamadas builtin.
- Autocomplete com ajuda rica:
  - funcoes mostram assinatura, uso basico e descricao;
  - builtins e stdlib ja trazem descricoes curtas;
  - funcoes do projeto usam comentarios `--` imediatamente acima da declaracao quando existirem.
- Autocomplete da stdlib:
  - `import std.io as io` permite `io.` sugerir `print`, `read_line`, `read_all`, `input`, `output` e `stderr`;
  - atalhos como `print()` inserem a chamada qualificada `io.print(...)`, sem criar import global escondido.
- Autocomplete em `import ...`:
  - sugere módulos `std.*`;
  - sugere namespaces de módulos `.zt` indexados no workspace, como `app.util` ou `borealis.game`.
- Autocomplete contextual depois de `.`:
  - `list<T>.` sugere `get(index)`.
  - `map<K,V>.` sugere `get(key)`.
  - `alias.` sugere membros publicos do namespace importado por `import ... as alias`.
  - `core.Error.` sugere `code`, `message` e `context`.
  - `Struct.` em variavel tipada sugere campos declarados da struct.
  - `objeto.` sugere metodos `apply` do tipo da variavel quando o tipo puder ser resolvido.
- Hover de simbolos declarados no arquivo atual.
- Hover de simbolos declarados em arquivos `.zt` indexados no workspace.
- Go-to-definition para simbolos declarados em arquivos `.zt` indexados no workspace.
- Find References para simbolos top-level indexados, incluindo usos via `alias.membro`.
- Rename Symbol para simbolos top-level indexados.
- Signature Help para chamadas diretas, `alias.func(...)` e `objeto.metodo(...)` quando o LSP consegue resolver o tipo.
- Semantic Tokens para highlight semantico leve.
- Outline do arquivo, com structs, fields, traits, metodos, enums, variants, funcoes, consts e vars.
- Busca de simbolos do workspace pelo comando nativo do VSCode.
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
- Hover, completion, go-to-definition, references, rename e signature help cross-file usam `namespace`, `import ... as ...` e `public` para reduzir sugestoes fora de contexto.
- Outline e busca de simbolos usam o indice em memoria da extensao.
- A resolucao ainda nao substitui o typechecker completo do projeto. Para erros definitivos, use `zt check`.
- Conversoes sugeridas pelo autocomplete seguem a regra da linguagem: `int(...)`, `float(...)` e variantes numericas exigem fonte numerica.
- Signature Help cobre chamadas diretas, `alias.func(...)` e metodos `apply` resolvidos por tipo.
- References e Rename cobrem simbolos top-level indexados. Variaveis locais ainda nao entram nesse corte.
- Semantic Tokens destaca keywords, tipos, funcoes, variaveis, propriedades, namespaces, strings e numeros.
- O autocomplete de membro sugere apenas recursos aceitos pelo compilador atual. Por isso `list<T>.append(...)` e `list<T>.prepend(...)` nao aparecem enquanto nao forem suportados semanticamente.
- O LSP usa sincronizacao full-document.
- A extensao beta ainda nao esta publicada no Marketplace.
- Os comandos `check/build/run` usam o terminal integrado do VSCode.
