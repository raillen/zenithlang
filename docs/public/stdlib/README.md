# Standard Library Guides

> Guias publicos da stdlib Zenith.
> Audience: user, package-author
> Surface: public
> Status: current

## Estado atual

A stdlib ainda esta em fase alpha.

Use apenas APIs documentadas e validadas por testes quando escrever exemplos publicos.

## Modulos comuns

- `std.io`: entrada e saída básica.
- `std.text`: helpers seguros de texto do corte alpha.
- `std.bytes`: base para dados binarios.
- `std.fs`: filesystem sincronico.
- `std.fs.path`: operações de caminho.
- `std.json`: JSON.
- `std.math`: matemática básica.
- `std.regex`: regex simples para validar e buscar texto.
- `std.random`: random alpha, com cuidado para estado compartilhado.
- `std.concurrent`: cópia explícita para fronteiras de concorrência.
- `std.test`: fail, skip e comparacoes simples para testes.
- `std.list`: helpers pequenos para `list<T>`.
- `std.map`: helpers pequenos para `map<K,V>`.
- `std.set`: helpers pequenos para `set<T>`.

## Testes pequenos com `std.test`

```zt
import std.test as test

attr test
public func score_starts_at_zero() -> void
    const score: int = 0
    test.equal_int(score, 0)
    test.is_true(score == 0)
end
```

Quando uma comparacao falha, prefira helpers que mostram esperado e recebido:

- `test.equal_int(actual, expected)`
- `test.equal_text(actual, expected)`
- `test.not_equal_int(actual, expected)`
- `test.not_equal_text(actual, expected)`

## Regra de uso

Prefira exemplos pequenos.

Se uma API ainda não esta suportada semanticamente pelo compilador, não coloque como guia publico.

## Referências

- Texto e arquivos: `docs/public/stdlib/text-and-files.md`.
- Validação, matemática e random: `docs/public/stdlib/validation-math-random.md`.
- Regex: `docs/public/stdlib/regex.md`.
- Sets: `docs/public/stdlib/set.md`.
- Referência da stdlib: `docs/reference/stdlib/README.md`.
- KB da stdlib: `docs/reference/zenith-kb/stdlib.md`.
- Modelo normativo: `language/spec/stdlib-model.md`.
- ZDoc da stdlib: `stdlib/zdoc/`.
