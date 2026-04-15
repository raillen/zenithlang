# Especificacao: Gramatica da Trilha Ativa

Este documento descreve a gramatica implementada hoje na trilha ativa em Lua. Ele nao tenta cobrir ideias editoriais antigas; cobre o que o parser aceita agora.

## 1. Palavras-chave reservadas

### Declaracao

var, const, global, func, async, struct, enum, trait, apply, type, union, extern, namespace, import, pub

### Fluxo

if, elif, else, match, case, while, for, in, repeat, times, break, continue, return, check, attempt, rescue, await

### Semantica

and, or, not, where, validate, is, as

### Literais especiais

true, false, null, self, it

### Escape

native, lua

## 2. Operadores relevantes

Da menor para a maior precedencia:

1. atribuicao: =, +=, -=, *=, /=
2. logicos: or, and
3. comparacao: ==, !=, <, <=, >, >=, is, as
4. soma e concatenacao: +, -, ..
5. multiplicativos: *, /, %
6. unarios: not, -, #, ?, !
7. acesso e chamada: ., [], ()

## 3. Declaracoes principais

~~~ebnf
declaration
  ::= attribute_block* visibility? (
          var_decl
        | const_decl
        | global_decl
        | func_decl
        | async_func_decl
        | struct_decl
        | enum_decl
        | trait_decl
        | apply_decl
        | type_alias_decl
        | union_decl
        | extern_decl
        | import_decl
        | namespace_decl
      )

visibility
  ::= "pub"
~~~

## 4. Atributos de declaracao

A forma normativa e #[...].

~~~ebnf
attribute_block
  ::= "#[" attribute_item ("," attribute_item)* "]"

attribute_item
  ::= qualified_name
   |  qualified_name "(" argument_list? ")"

qualified_name
  ::= IDENTIFIER ("." IDENTIFIER)*
~~~

Compatibilidade:

- @atributo ainda e aceito por compatibilidade
- a forma legada emite warning ZT-W003
- a gramatica oficial considera apenas #[...] como forma normativa

## 5. Structs e contratos de campo

~~~ebnf
struct_decl
  ::= "struct" IDENTIFIER generic_params? NEWLINE
      struct_member*
      "end"

struct_member
  ::= attribute_block* visibility? (
          field_decl
        | func_decl
        | async_func_decl
      )

field_decl
  ::= IDENTIFIER ":" type_expr field_initializer? field_contract*

field_initializer
  ::= "=" expression

field_contract
  ::= where_clause
   |  validate_clause

where_clause
  ::= "where" expression

validate_clause
  ::= "validate" validator_ref ("," validator_ref)*

validator_ref
  ::= qualified_name
   |  qualified_name "(" argument_list? ")"
~~~

Notas normativas:

- where e validate podem aparecer em qualquer ordem
- validate e exclusivo de campo de struct
- validate e reescrito para predicados booleanos sobre it

## 6. Funcoes, metodos e self

~~~ebnf
func_decl
  ::= "func" IDENTIFIER generic_params? "(" parameter_list? ")" return_clause? block "end"

async_func_decl
  ::= "async" "func" IDENTIFIER generic_params? "(" parameter_list? ")" return_clause? block "end"

parameter
  ::= IDENTIFIER (":" type_expr)?

return_clause
  ::= ("->" | ":") type_expr
~~~

self e keyword reservada. No dominio de expressao, @campo e sugar para self.campo.

~~~ebnf
self_field_expr
  ::= "@" IDENTIFIER
~~~

## 7. Traits e apply

~~~ebnf
trait_decl
  ::= "trait" IDENTIFIER generic_params? NEWLINE
      trait_member*
      "end"

apply_decl
  ::= "apply" IDENTIFIER generic_args? "to" IDENTIFIER NEWLINE
      func_decl*
      "end"
~~~

A sintaxe implementada hoje e apply Trait to Struct.

## 8. Genericos e restricoes

~~~ebnf
generic_params
  ::= "<" generic_param ("," generic_param)* ">"

generic_param
  ::= IDENTIFIER ("where" IDENTIFIER "is" type_expr)?
~~~

Forma normativa:

~~~zt
func render<T where T is Printable>(value: T) -> text
~~~

## 9. Match e padroes

~~~ebnf
match_stmt
  ::= "match" expression NEWLINE
      match_case+
      else_clause?
      "end"

match_case
  ::= "case" pattern_list (":" | "=>") case_body

pattern_list
  ::= pattern ("," pattern)*

pattern
  ::= literal
   |  "_"
   |  IDENTIFIER
   |  qualified_name
   |  qualified_name "(" pattern_list? ")"
   |  IDENTIFIER "{" field_pattern_list? "}"
~~~

Padroes qualificados como Color.Red fazem parte da trilha ativa.

## 10. Indexacao e slices

~~~ebnf
index_expr
  ::= expression "[" expression "]"
   |  expression "[" range_expr "]"

range_expr
  ::= expression ".." expression
~~~

Semantica associada:

- listas e texto sao 1-based
- maps usam a chave declarada
- slices e indexacao de sequencia podem gerar ZT-R011 em runtime quando fora dos limites
