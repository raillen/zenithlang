# Limpeza com `using`

> Audience: user
> Surface: public
> Status: current

## Ideia central

Use `using` quando um valor precisa ter um fim claro.

Ele ajuda em casos como:

- abrir e fechar um recurso;
- registrar uma acao de limpeza;
- garantir ordem de limpeza em retornos antecipados.

## Forma com bloco

```zt
using handle = open_handle()
    use(handle)
end
```

Nesta forma, `handle` existe somente dentro do bloco.

## Forma plana

```zt
using handle = open_handle()
use(handle)
return 0
```

Nesta forma, `handle` existe ate o fim do bloco atual.

Em uma funcao, isso normalmente significa ate o fim da funcao.

Dentro de um loop, isso significa ate o fim da iteracao atual.

## Limpeza explicita

```zt
using handle = open_handle() then close_handle(handle)
use(handle)
```

A expressao depois de `then` roda quando o escopo termina.

Ela tambem roda antes de:

- `return`;
- propagacao com `?`;
- `break`;
- `continue`.

## Ordem

Quando existem varios `using` ativos, a limpeza e LIFO.

Isso significa:

1. o ultimo `using` ativo limpa primeiro;
2. o primeiro `using` ativo limpa por ultimo.

```zt
using first = 1 then cleanup(first)
using second = 2 then cleanup(second)

return 0
```

Neste exemplo, `cleanup(second)` roda antes de `cleanup(first)`.

## Regra importante

Se o inicializador falha com `?`, a limpeza desse novo `using` ainda nao esta ativa.

```zt
using value = load_value()? then cleanup(value)
```

Se `load_value()?` propagar erro, `cleanup(value)` nao roda, porque `value` nao foi criado.
