# Especificacao: Sistema de Tipos da Trilha Ativa

Este documento descreve o sistema de tipos que o binder e o runtime efetivamente sustentam hoje.

## 1. Tipos basicos

Tipos primitivos da trilha ativa:

- int
- float
- text
- bool
- any
- null
- void

Regras importantes:

- int pode ser promovido para float
- any continua sendo escape hatch de interoperabilidade
- null existe por compatibilidade, mas nao e a forma idiomatica de ausencia

## 2. Tipos compostos

Tipos compostos implementados:

- list<T>
- map<K, V>
- grid<T>
- Optional<T>
- Outcome<T, E>
- tipos anulaveis T?
- tipos uniao

## 3. Nulabilidade

T? representa anulabilidade no sistema de tipos.

Politica atual:

- var x: text? = null e permitido com warning ZT-W001
- var x: text = null gera ZT-S100

Preferencia de modelagem:

- use Optional<T> para ausencia semantica
- use Outcome<T, E> para sucesso ou falha

## 4. Optional e Outcome

### Optional<T>

Representa valor presente ou ausente.

Surface real da trilha ativa:

- Present(value: T)
- Empty
- is_present()
- is_empty()
- unwrap()
- unwrap_or(default)

### Outcome<T, E>

Representa sucesso ou falha explicitos.

Surface real da trilha ativa:

- Success(value: T)
- Failure(error: E)
- is_success()
- is_failure()
- unwrap()

No runtime atual, Outcome tambem participa da familia de helpers de presenca:

- Success e tratado como presente
- Failure e tratado como vazio para is_empty() e unwrap_or(default)

## 5. Structs e contratos de campo

Campos de struct aceitam contratos por where e validate.

~~~zt
struct Product
    name: text validate validation.non_empty
    price: float where it > 0.0
    stock: int validate validation.min_value(0) where it <= 9999
end
~~~

Regra semantica:

- where recebe qualquer expressao booleana valida
- validate recebe referencias a predicados booleanos reutilizaveis
- validate e reescrito para chamadas sobre it

## 6. std.validation

A trilha ativa possui um modulo oficial de validadores declarativos:

- min_value
- max_value
- between
- one_of
- non_empty
- not_blank
- len_min
- len_max
- len_between
- matches
- email
- url
- uuid

Uso tipico:

~~~zt
import std.validation as validation

struct User
    age: int validate validation.min_value(18), validation.max_value(120)
    email: text validate validation.email
end
~~~

## 7. Genericos e restricoes

Genericos sao parte formal do sistema de tipos. A sintaxe implementada para restricao e:

~~~zt
func render<T where T is Printable>(value: T) -> text
~~~

Propriedades atuais:

- parametros de tipo existem formalmente no binder
- especializacao profunda esta implementada
- genericos sao invariantes por padrao

## 8. Traits e apply

Traits declaram comportamento. apply Trait to Struct implementa o contrato.

~~~zt
trait Printable
    func to_text(self) -> text
end

apply Printable to User
    func to_text(self) -> text
        return @name
    end
end
~~~

## 9. Conversoes e metodo virtual

Alguns comportamentos aparecem como metodos virtuais conhecidos pelo binder e pelo runtime:

- len()
- split()
- push(value)
- pop()
- keys()
- is_present()
- is_empty()
- unwrap()
- to_text()

Eles fazem parte da surface real da linguagem, mesmo quando a declaracao formal em std.core ainda e parcial.
