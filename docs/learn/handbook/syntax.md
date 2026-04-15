# Guia de Sintaxe da Trilha Ativa

Este guia descreve a surface syntax implementada hoje na trilha ativa em Lua. Quando houver diferenca entre documentos antigos e este guia, considere este arquivo e o RFC 001 como referencia normativa.

## 1. Declaracoes basicas

Use var para valores mutaveis, const para constantes locais e global apenas quando o simbolo realmente precisa entrar no escopo global da VM.

~~~zt
var nivel: int = 1
const APP_NAME: text = "Zenith"
global cache: map<text, any> = {}
~~~

Tipos podem ser explicitados com :. A trilha ativa tambem aceita int? para nulabilidade.

~~~zt
var total: int = 10
var maybe_name: text? = Empty
~~~

## 2. Colecoes e indexacao

Colecoes principais:

- list<T> para sequencias ordenadas
- map<K, V> para associacao por chave
- grid<T> para grade 2D

~~~zt
var itens: list<text> = ["espada", "escudo"]
var status: map<text, text> = { "mode": "debug" }
~~~

Regras de acesso:

- player.nome acessa campo de struct ou membro conhecido
- status["mode"] acessa chave de mapa
- itens[1] acessa lista
- "Zenith"[1] acessa texto

Na trilha ativa, listas e texto sao 1-based.

- lista[1] e o primeiro elemento
- lista[0] e texto[0] geram warning ZT-W002
- acesso invalido em runtime gera ZT-R011
- mapas nao seguem filosofia 1-based; a chave usada e a chave declarada

~~~zt
var nomes = ["ana", "bia", "caio"]
var primeiro = nomes[1]

var config: map<text, text> = { "status": "ok" }
var s = config["status"]
~~~

## 3. Funcoes e metodos

Funcoes usam -> ou : para tipo de retorno na trilha ativa.

~~~zt
func soma(a: int, b: int) -> int
    return a + b
end
~~~

Metodos vivem dentro de struct e podem usar self. O atalho @campo e sugar para self.campo.

~~~zt
struct Player
    hp: int

    func reset(self)
        @hp = 100
    end
end
~~~

Regras importantes:

- @campo so vale dentro de metodo
- @metodo(args) vira chamada em self
- usar self ou @campo fora de metodo gera ZT-S201

## 4. Structs, traits e apply

struct modela dados. trait declara comportamento. apply Trait to Struct implementa o contrato.

~~~zt
struct Player
    nome: text
    hp: int where it >= 0
end

trait Healable
    func heal(self, amount: int)
end

apply Healable to Player
    func heal(self, amount: int)
        @hp = @hp + amount
    end
end
~~~

Observacao:

- a sintaxe implementada hoje e apply Trait to Struct
- exemplos antigos com apply Trait for Struct estao desatualizados

## 5. Atributos de declaracao

A forma normativa para atributos de declaracao e #[...].

~~~zt
#[windows, deprecated("use new_main")]
func main()
end
~~~

Campos tambem aceitam atributos:

~~~zt
struct User
    #[doc.hidden]
    token: text
end
~~~

Compatibilidade legada:

- @atributo ainda e aceito no parser
- a forma legada emite warning ZT-W003
- documentacao oficial deve preferir sempre #[...]

## 6. Contratos de campo: where e validate

Campos de struct aceitam where e validate, inclusive no mesmo campo e em qualquer ordem.

~~~zt
import std.validation as validation

struct User
    age: int validate validation.min_value(18), validation.max_value(120)
    name: text validate validation.non_empty where not text.contains(it, "admin")
end
~~~

Regra pratica:

- validate para predicados reutilizaveis
- where para expressoes booleanas livres

validate baixa para chamadas sobre it. Exemplo:

~~~zt
age: int validate validation.min_value(18)
~~~

equivale semanticamente a:

~~~zt
age: int where validation.min_value(it, 18)
~~~

## 7. Match e padroes

match suporta literais, wildcard, variantes e padroes qualificados como Color.Red.

~~~zt
match cor
    case Color.Red => print("vermelho")
    case Color.Blue => print("azul")
    case _ => print("outra")
end
~~~

## 8. Optional, Outcome e fallback

O modelo recomendado de ausencia e falha e:

- Optional<T> para valor ausente
- Outcome<T, E> para sucesso ou falha explicitos

Helpers disponiveis na trilha ativa:

- is_present()
- is_empty()
- unwrap()
- unwrap_or(default)

~~~zt
if user_opt.is_present()
    print(user_opt.unwrap())
end

var user = user_opt.unwrap_or("guest")
~~~

O operador ? continua disponivel para propagacao.

## 9. UFCS virtual

Alguns metodos sao resolvidos pelo binder e pelo runtime, mesmo sem declaracao formal completa no prelude:

- len()
- split()
- push(value)
- pop()
- keys()
- is_present()
- is_empty()
- unwrap()
- to_text()

Exemplos:

~~~zt
var partes = "a,b,c".split()
var n = partes.len()
var txt = 42.to_text()
~~~

Na trilha ativa, split() sem argumento usa "," como separador padrao.

## 10. Genericos e restricoes

Genericos aceitam restricoes com where T is Trait.

~~~zt
trait Printable
    func to_text(self) -> text
end

func render<T where T is Printable>(value: T) -> text
    return value.to_text()
end
~~~

Esta e a sintaxe implementada hoje. Formas como T: Trait nao sao a forma normativa da trilha ativa.
