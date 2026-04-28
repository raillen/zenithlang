# Zenith VSCode Beta

Extensao beta para arquivos `.zt`.

## Recursos

- Diagnostics em tempo real via Compass LSP.
- Completion global, contextual, por import, por membro e por builtin, com assinatura e documentacao em Markdown.
- Hover de simbolos top-level indexados.
- Go-to-definition para simbolos top-level indexados, incluindo outro arquivo.
- References para simbolos top-level indexados, incluindo `alias.membro`.
- Rename para simbolos top-level indexados.
- Signature Help para chamadas diretas, `alias.func(...)` e metodos `apply` resolvidos por tipo.
- Semantic Tokens baseados no lexer oficial para highlight semantico leve.
- Outline do arquivo via Document Symbols.
- Busca de simbolos do workspace via Workspace Symbols.
- Formatacao pelo formatter oficial do compilador.
- `Zenith: New File` cria `.zt` com `namespace` calculado pelo caminho em `source.root`.
- Arquivos `.zt` vazios criados dentro de `source.root` recebem `namespace` automaticamente.
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
- `zenith.files.autoNamespaceOnCreate`: ativa ou desativa o `namespace` automatico em `.zt` vazio.

## Limites da beta

- O indice atual e em memoria sobre arquivos `.zt` sincronizados.
- Diagnostics usam sincronizacao full-document.
- Builtins sugeridos incluem `fmt`, `len()`, `to_text()`, `check()`, `todo()`, `unreachable()`, `panic()` e conversoes numericas como `int()` e `float()`.
- Completion mostra assinatura, uso basico e descricao para builtins, funcoes da stdlib e funcoes do projeto.
- Comentarios `--` imediatamente acima de uma funcao enriquecem a ajuda exibida pelo autocomplete e pelo Signature Help.
- Imports da stdlib agora expõem membros públicos no autocomplete. Exemplo: depois de `import std.io as io`, `io.` sugere `print`, `read_line`, `read_all`, `input`, `output` e `stderr`.
- Atalhos de stdlib importada aparecem de forma qualificada: selecionar `print()` insere `io.print(...)`, preservando a regra de que imports continuam qualificados.
- Signature Help cobre chamadas diretas, `alias.func(...)` e `objeto.metodo(...)` para metodos `apply` indexados.
- References e Rename cobrem simbolos top-level indexados.
- Semantic Tokens usa o lexer oficial para a sintaxe atual; a grammar TextMate continua sendo o fallback visual.
- O namespace automatico usa `zenith.ztproj` e `[source].root`; se o arquivo ficar fora de `source.root`, a extensao nao altera o conteudo.
- Comandos `check/build/run` executam no terminal integrado.
