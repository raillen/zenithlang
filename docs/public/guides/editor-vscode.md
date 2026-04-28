# Editor VSCode para Zenith

Este guia cobre o uso local da extensão beta `tools/vscode-zenith`.

## O que funciona neste corte

- Diagnostics em tempo real para arquivos `.zt` abertos.
- Autocomplete de palavras-chave, tipos, builtins, snippets básicos, símbolos do arquivo aberto, variaveis locais e parametros.
- Autocomplete de builtins:
  - `fmt` insere `fmt "..."`;
  - `int()`, `float()`, `int64()` e outros tipos numéricos inserem conversão explícita;
  - `len()`, `to_text()`, `check()`, `todo()`, `unreachable()` e `panic()` aparecem como chamadas builtin.
- Autocomplete com ajuda rica:
  - funções mostram assinatura, uso básico e descrição;
  - builtins e stdlib já trazem descrições curtas;
  - funções do projeto usam comentarios `--` imediatamente acima da declaração quando existirem.
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
  - `objeto.` sugere métodos `apply` do tipo da variavel quando o tipo puder ser resolvido.
- Hover de símbolos declarados no arquivo atual.
- Hover de símbolos declarados em arquivos `.zt` indexados no workspace.
- Go-to-definition para símbolos declarados em arquivos `.zt` indexados no workspace.
- Find References para símbolos top-level indexados, incluindo usos via `alias.membro`.
- Rename Symbol para símbolos top-level indexados.
- Signature Help para chamadas diretas, `alias.func(...)` e `objeto.metodo(...)` quando o LSP consegue resolver o tipo.
- Semantic Tokens baseados no lexer oficial para highlight semântico leve.
- Outline do arquivo, com structs, fields, traits, métodos, enums, variants, funções, consts e vars.
- Busca de símbolos do workspace pelo comando nativo do VSCode.
- Formatacao pelo formatter oficial usado pelo compilador.
- Comandos rápidos no editor:
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

## Passo 2: abrir a extensão em modo desenvolvimento

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

Se preferir não formatar ao salvar, mantenha `editor.formatOnSave` como `false`.

## Configuracoes úteis

```json
{
  "zenith.lsp.path": "C:/caminho/para/zt-lsp.exe",
  "zenith.cli.path": "C:/caminho/para/zt.exe"
}
```

Use essas configurações quando o VSCode não encontrar os binarios automaticamente.

## Como validar rápido

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

- O índice do workspace e textual/in-memory: a extensão indexa arquivos `.zt` do workspace e atualiza em create/change/delete.
- Hover, completion, go-to-definition, references, rename e signature help cross-file usam `namespace`, `import ... as ...` e `public` para reduzir sugestoes fora de contexto.
- Outline e busca de símbolos usam o índice em memória da extensão.
- A resolucao ainda não substitui o typechecker completo do projeto. Para erros definitivos, use `zt check`.
- Conversoes sugeridas pelo autocomplete seguem a regra da linguagem: `int(...)`, `float(...)` e variantes numéricas exigem fonte numérica.
- Signature Help cobre chamadas diretas, `alias.func(...)` e métodos `apply` resolvidos por tipo.
- References e Rename cobrem símbolos top-level indexados. Variaveis locais ainda não entram nesse corte.
- Semantic Tokens destaca a sintaxe atual reconhecida pelo lexer: keywords, tipos, funções, variaveis, propriedades, namespaces, strings e números.
- O autocomplete de membro sugere apenas recursos aceitos pelo compilador atual. Por isso `list<T>.append(...)` e `list<T>.prepend(...)` não aparecem enquanto não forem suportados semanticamente.
- O LSP usa sincronização full-document.
- A extensão beta ainda não esta publicada no Marketplace.
- Os comandos `check/build/run` usam o terminal integrado do VSCode.
