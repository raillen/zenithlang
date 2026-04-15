# Mapa de Intencoes Visuais

Este documento define a leitura semantica dos simbolos da trilha ativa.

## 1. Blocos e estrutura

| Simbolo | Leitura pratica |
| :-- | :-- |
| end | encerra o bloco atual |
| : | ancora tipagem de parametro, campo ou declaracao |
| -> | ancora o tipo de retorno |
| => | caso curto de match |

## 2. Dados e fluxo

| Simbolo | Leitura pratica |
| :-- | :-- |
| ? | propaga Optional ou Outcome |
| ! | extrai de forma estrita |
| .. | range ou slice |
| # | operador de tamanho |
| [] | indexacao ou acesso por chave |

## 3. @, #[...] e validate

Esses tres pontos nao devem mais ser confundidos.

### @

@campo e acucar sintatico para self.campo.

Leitura correta:

- @hp = campo hp do proprio objeto
- @reset() = chame reset em self

@ nao e mais a sintaxe normativa de atributo de declaracao.

### #[...]

#[...] marca atributos de declaracao.

Leitura correta:

- #[windows] = esta declaracao tem atributo windows
- #[deprecated("msg")] = esta declaracao esta marcada como deprecated

### validate

validate declara validadores reaproveitaveis para campos de struct.

Leitura correta:

- validate min_value(18) = o campo deve satisfazer o predicado min_value

validate nao e um bloco livre; ele e sugar para composicao booleana sobre it.

## 4. Acesso por ponto e por indice

| Forma | Intencao |
| :-- | :-- |
| obj.campo | membro estrutural conhecido |
| map["status"] | chave de mapa |
| lista[1] | elemento de sequencia |

Regra cognitiva:

- . comunica estrutura
- [] comunica chave ou posicao

## 5. Filosofia de indexacao

Na trilha ativa:

- listas e texto sao 1-based
- mapas nao sao 1-based; usam a chave real
- 0 em sequencia e sinal de erro conceitual e pode gerar ZT-W002
- acesso invalido em runtime usa ZT-R011
