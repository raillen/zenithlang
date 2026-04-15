# Modulo std.core

std.core e a base semantica da linguagem. Seus simbolos formam a prelude da trilha ativa e convivem com alguns helpers de runtime usados por UFCS.

## Funcoes globais

| API | Descricao |
| :-- | :-- |
| print(value: any) | Escreve no console. |
| error(message: text) | Interrompe a execucao atual. |
| assert(condition: bool, message: text) | Falha se a condicao for falsa. |
| panic(message: text) | Variante semantica para falha irrecuperavel. |

## Tipos algebraicos

### Optional<T>

Representa presenca ou ausencia de valor.

~~~zt
var nome: Optional<text> = Present("Zenith")

match nome
    case Present(valor):
        print("Nome: " + valor)
    case Empty:
        print("Sem valor")
end
~~~

Casos disponiveis:

- Present(value: T)
- Empty

Helpers efetivos da trilha ativa:

- opt.is_present() -> bool
- opt.is_empty() -> bool
- opt.unwrap() -> T
- opt.unwrap_or(default) -> T

### Outcome<T, E>

Representa sucesso ou falha explicitos.

~~~zt
var res = fs.read_text_file("config.json")

match res
    case Success(txt):
        print(txt)
    case Failure(err):
        print("Falha: " + err)
end
~~~

Casos disponiveis:

- Success(value: T)
- Failure(error: E)

Helpers declarados ou efetivos:

- res.is_success() -> bool
- res.is_failure() -> bool
- res.unwrap() -> T
- res.unwrap_or(default) -> T no runtime atual
- res.is_present() -> bool no runtime atual
- res.is_empty() -> bool no runtime atual

Observacao importante:

- no runtime atual, Success e tratado como presente
- no runtime atual, Failure e tratado como vazio para fallback

## UFCS virtual da trilha ativa

Algumas APIs nao precisam existir como declaracao formal completa em std.core; elas sao resolvidas pelo binder e pelo runtime como metodos virtuais.

### Optional e Outcome

- is_present()
- is_empty()
- unwrap()
- unwrap_or(default)

### Collections

- len()
- push(value)
- pop()
- keys()

### Texto

- split()
- len()

### Primitivos

- to_text()

## Operador ?

O operador ? funciona com Optional e Outcome dentro de funcoes compativeis.

~~~zt
func carregar() -> Outcome<text, text>
    var txt = fs.read_text_file("app.txt")?
    return Success(txt)
end
~~~

## Politica de null

A trilha ativa ainda possui null por compatibilidade, mas o uso direto e desencorajado.

Regra atual:

- T? = null: permitido com warning ZT-W001
- T = null: erro semantico ZT-S100

Preferencia idiomatica:

- ausencia de valor: Optional.Empty
- falha explicita: Outcome.Failure

## Observacoes

- Esta pagina descreve o comportamento real da trilha ativa.
- APIs antigas como type_of e is_null nao fazem parte da interface atual.
- to_text() existe hoje como UFCS virtual de runtime, nao como declaracao formal classica de prelude.
