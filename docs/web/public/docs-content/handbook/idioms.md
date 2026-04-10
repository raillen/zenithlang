# Idiomatismo e Boas Práticas

Este guia resume o estilo recomendado para o Zenith atual, com foco em legibilidade, previsibilidade e baixo atrito cognitivo.

## 1. Nomeie com intenção

- Variáveis e funções: `snake_case`
- Tipos, `struct`, `enum` e `trait`: `PascalCase`
- Constantes: `SCREAMING_SNAKE_CASE`

```zt
const MAX_RETRIES: int = 3

struct ApiClient
    pub base_url: text
end

func carregar_usuario() -> text
    return "ok"
end
```

## 2. Prefira tipos explícitos

Evite depender de inferência implícita para nomes importantes.

```zt
var retries: int = 0
var status: text | int = "ok"
```

Quando quiser carregar significado de domínio, use `type`:

```zt
type UserId = text
```

Quando quiser modelar mais de um caso válido, use `union` ou `enum`:

```zt
union SearchResult = text | int
```

## 3. Modele falhas no tipo

Use `Optional` e `Outcome` em vez de esconder erro em convenções soltas.

```zt
func carregar() -> Outcome<text, text>
    return fs.read_text_file("config.json")
end
```

Para consumo, prefira `match`, `or` ou `?`:

```zt
var user = os.get_env_variable("USERNAME") or "desconhecido"
```

## 4. Use `where` para invariantes de dados

```zt
struct Item
    pub nome: text
    pub quantidade: int where it >= 0
end
```

Se um campo não pode assumir qualquer valor, deixe isso perto da definição do tipo.

## 5. Composição antes de herança

O caminho idiomático do Zenith é `trait` + `apply`.

```zt
trait Renderable
    pub func render() -> text
end

apply Renderable to Card
    pub func render() -> text
        return @title
    end
end
```

## 6. Use `@campo` só dentro de métodos

Dentro de `struct` e blocos `apply`, `@campo` melhora legibilidade. Fora disso, prefira nomes explícitos.

```zt
pub func render() -> text
    return "Olá, " + @nome
end
```

## 7. Separe terminal, arquivos e sistema

- `std.io` para terminal
- `std.fs` para arquivos
- `std.json` para serialização
- `std.os` para ambiente e shell
- `std.os.process` para processos em background

Essa separação torna o código mais fácil de testar e de entender.

## 8. Use `native lua` como fronteira, não como padrão

`native lua` é útil para interoperabilidade, mas o código idiomático de Zenith mantém a maior parte da regra de negócio em Zenith puro.

## 9. Prefira exemplos verticais

Zenith foi desenhado para leitura descendente. Quebre chamadas e decisões em blocos curtos.

```zt
var resultado: int = inicial
    .step1(5)
    .step2()
```

## 10. Trate áreas em transição com cautela

- `state`, `computed` e `watch` existem, mas a ergonomia do runtime ainda está sendo refinada.
- O pipeline async está em evolução; `async`, `await`, `time.wait()` e `std.os.process.wait()` já fazem parte da superfície pública, mas ainda recebem estabilização interna.
