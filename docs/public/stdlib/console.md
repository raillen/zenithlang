# Modulo `std.console`

`std.console` e a camada para programas interativos de terminal.

Use `std.io` para streams, pipes e automacao.
Use `std.console` para conversar com uma pessoa no terminal.

## Saida com linha

```zt
import std.console as console

func main() -> result<void, core.Error>
    console.write_line("Pronto.")?
    console.error_line("Aviso no stderr.")?
    return success()
end
```

## Entrada interativa

```zt
const name: text = console.prompt("Nome: ")?
const ok: bool = console.confirm("Continuar?", default_value: true)?
```

`pause`, `prompt` e `confirm` podem bloquear.
Use apenas em apps interativos.

## Terminal

```zt
const current: console.Size = console.size()
const interactive: bool = console.is_terminal("stdout")
const key: optional<text> = console.read_key()?
```

`read_key` nao bloqueia.
Retorna `none` quando nao ha tecla disponivel ou quando a entrada nao e um terminal.

## Cores e estilos

```zt
console.color("green")?
console.style("bold")?
console.write_line("OK")?
console.reset_style()?
```

Cores aceitas: `default`, `black`, `red`, `green`, `yellow`, `blue`, `magenta`, `cyan`, `white` e variantes `bright_*`.

Estilos aceitos: `reset`, `bold`, `dim`, `italic`, `underline` e `reverse`.

## Limpar tela

```zt
console.clear()?
```

Use com cuidado.
Em logs, pipes e testes automatizados, prefira saida textual simples.

