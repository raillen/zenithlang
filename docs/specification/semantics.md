# Especificacao: Semantica da Trilha Ativa

Este documento descreve o comportamento semantico realmente aplicado hoje pelo binder, pelo lowering e pelo runtime.

## 1. Resolucao de nomes e escopo

A trilha ativa usa escopo lexico estatico.

- simbolos pub sao exportaveis
- simbolos sem pub sao locais ao modulo
- shadowing continua permitido em escopos internos

## 2. self e @campo

self so pode ser usado dentro de metodo. O mesmo vale para @campo, que e sugar de self.campo.

Diagnostico associado:

- usar self ou @campo fora de metodo gera ZT-S201

## 3. Contratos de campo

Campos de struct suportam dois mecanismos semanticos complementares:

- where para expressao booleana livre
- validate para predicados booleanos reaproveitaveis

where e validate podem coexistir e sao combinados com and.

Exemplo:

~~~zt
age: int validate validation.min_value(18) where it <= 120
~~~

Leitura semantica equivalente:

~~~zt
age: int where validation.min_value(it, 18) and it <= 120
~~~

## 4. Null, Optional e Outcome

Politica atual:

- null ainda existe por compatibilidade
- T? = null gera erro semantico ZT-S106
- T = null gera erro ZT-S106 e tambem ZT-S100

Modelos recomendados:

- Optional<T> para ausencia
- Outcome<T, E> para sucesso ou falha

No runtime atual:

- is_present() considera Present e Success como presentes
- is_empty() considera Empty e Failure como vazios
- unwrap_or(default) funciona para Optional, Outcome e fallback geral

## 5. UFCS e metodos virtuais

A trilha ativa tem dois caminhos de UFCS:

- metodo real de struct ou trait
- metodo virtual conhecido pelo binder e resolvido no runtime

Surface virtual atual:

- len
- split
- push
- pop
- keys
- is_empty
- is_present
- unwrap
- to_text

Observacoes:

- UFCS so vale para instancias
- split() sem argumento usa "," no caminho virtual da trilha ativa

## 6. Indexacao

A filosofia semantica implementada hoje e:

- listas e texto sao 1-based
- mapas usam a chave real
- indice literal 0 em sequencia gera warning ZT-W002
- acesso invalido em runtime gera ZT-R011

A emissao de codigo escolhe helpers diferentes:

- lista: zt.index_seq
- texto: zt.index_text
- dinamico: zt.index_any
- slices: zt.slice

## 7. Genericos e restricoes

Restricoes genericas sao parseadas e tambem validadas semanticamente.

Sintaxe implementada:

~~~zt
func render<T where T is Printable>(value: T) -> text
~~~

A trilha ativa tambem sustenta:

- especializacao profunda
- validacao de restricao em chamadas e instanciacoes
- invariancia de genericos por padrao

## 8. Match e padroes

A trilha ativa suporta:

- wildcard _
- padroes literais
- padroes de variante
- padroes qualificados como Color.Red
- padroes estruturais basicos

## 9. Fallback com or

No codegen atual, o operador or desce para a base de unwrap_or. Isso preserva a semantica de fallback e default usada na trilha ativa.

Uso idiomatico:

~~~zt
var user = env_opt or "guest"
~~~

Para expressoes de controle mais complexas, if e match continuam sendo as ferramentas mais explicitas.
