# Módulo `std.json`

Este módulo cobre o subset estável de JSON no runtime atual.

Escopo atual:
- objeto JSON com chaves `text` e valores `text`
- representação em Zenith: `map<text,text>`

Fora do escopo por enquanto:
- arrays gerais
- números tipados
- bool/null com modelo rico
- árvore JSON tipada (`Value/Object/Array`)

## Constantes e Funções

### `parse`

```zt
public func parse(input: text) -> result<map<text,text>, core.Error>
```

Converte texto JSON para `map<text,text>`.

@param input Texto JSON.
@return Mapa em sucesso, ou erro em `core.Error`.

### `stringify`

```zt
public func stringify(value: map<text,text>) -> text
```

Converte `map<text,text>` para JSON compacto.

### `pretty`

```zt
public func pretty(value: map<text,text>, indent: int = 2) -> text
```

Converte `map<text,text>` para JSON formatado.

@param indent Espaços por nível (padrão: 2).

### `read`

```zt
public func read(file_path: text) -> result<map<text,text>, core.Error>
```

Lê o arquivo e faz parse em uma única chamada.

### `write`

```zt
public func write(file_path: text, value: map<text,text>) -> result<void, core.Error>
```

Serializa e escreve em arquivo.

