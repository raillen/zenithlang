# Módulo `std.fs`

Módulo síncrono de filesystem.
Caminhos são `text`.
Manipulação de path continua em `std.fs.path`.

## Constantes e Funções

### `Error`

```zt
public enum Error
    NotFound
    PermissionDenied
    AlreadyExists
    NotADirectory
    IsADirectory
    IOError
    InvalidPath
    Unknown
end
```

Erro tipado para operações de filesystem.

### `Metadata`

```zt
public struct Metadata
    size_bytes: int
    modified_at_ms: int
    created_at_ms: optional<int>
    is_file: bool
    is_dir: bool
end
```

Metadados expostos no alpha atual.
Timestamps usam Unix milliseconds.

### `read_text`

```zt
public func read_text(path: text) -> result<text, fs.Error>
```

Lê um arquivo texto UTF-8.

### `write_text`

```zt
public func write_text(path: text, content: text) -> result<void, fs.Error>
```

Escreve texto, substituindo o conteúdo anterior.

### `append_text`

```zt
public func append_text(path: text, content: text) -> result<void, fs.Error>
```

Acrescenta texto ao final do arquivo.

### `exists`

```zt
public func exists(path: text) -> result<bool, fs.Error>
```

Verifica se o caminho existe.

### `is_file`

```zt
public func is_file(path: text) -> result<bool, fs.Error>
```

Verifica se o caminho aponta para arquivo.

### `is_dir`

```zt
public func is_dir(path: text) -> result<bool, fs.Error>
```

Verifica se o caminho aponta para diretório.

### `create_dir`

```zt
public func create_dir(path: text) -> result<void, fs.Error>
```

Cria um diretório.

### `create_dir_all`

```zt
public func create_dir_all(path: text) -> result<void, fs.Error>
```

Cria um diretório e os país necessários.

### `list_dir`

```zt
public func list_dir(path: text) -> result<list<text>, fs.Error>
```

Lista os nomes das entradas de um diretório.

### `remove_file`

```zt
public func remove_file(path: text) -> result<void, fs.Error>
```

Remove um arquivo.

### `remove_dir`

```zt
public func remove_dir(path: text) -> result<void, fs.Error>
```

Remove um diretório vazio.

### `remove_dir_all`

```zt
public func remove_dir_all(path: text) -> result<void, fs.Error>
```

Remove um diretório recursivamente.

### `copy_file`

```zt
public func copy_file(from: text, to: text) -> result<void, fs.Error>
```

Copia um arquivo.

### `move`

```zt
public func move(from: text, to: text) -> result<void, fs.Error>
```

Move ou renomeia uma entrada.

### `metadata`

```zt
public func metadata(path: text) -> result<fs.Metadata, fs.Error>
```

Retorna metadados completos da entrada.

### `size`

```zt
public func size(path: text) -> result<int, fs.Error>
```

Retorna o tamanho em bytes.

### `modified_at`

```zt
public func modified_at(path: text) -> result<int, fs.Error>
```

Retorna o timestamp de modificação em Unix milliseconds.

### `created_at`

```zt
public func created_at(path: text) -> result<optional<int>, fs.Error>
```

Retorna o timestamp de criação em Unix milliseconds quando disponível.

