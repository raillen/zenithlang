# Zenith Examples

Colecao de exemplos curtos para demonstrar a linguagem no GitHub.

## Como rodar

Use `check`, `build` e `run` em qualquer exemplo:

```bash
./zt.exe check examples/hello-world/zenith.ztproj
./zt.exe build examples/hello-world/zenith.ztproj
./zt.exe run examples/hello-world/zenith.ztproj
```

## Exemplos

1. `hello-world`
   - `result<void, core.Error>` + `std.io`
2. `structs-and-match`
   - `struct`, `enum`, `match`
3. `optional-and-result`
   - `optional<T>`, `result<T, E>`, operador `?`
4. `multifile-imports`
   - `namespace`, `import`, `public func`
5. `std-json`
   - `std.json.parse`, `std.json.stringify`, `std.json.pretty`
6. `extern-c-puts`
   - bloco `extern c` com chamada nativa

## Dica para README do GitHub

Para vitrine rapida, use:

1. `hello-world`
2. `structs-and-match`
3. `std-json`

