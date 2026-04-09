# Zenith Implementation: Semântica e Lowering

## Resumo rápido
Este módulo explica como o compilador transforma uma AST sintática em significado validado e, depois, em uma IR simplificada pronta para o backend Lua.

## Quando usar
Leia este documento quando estiver trabalhando com símbolos, escopos, tipos, regras de binding, análises semânticas ou transformações de lowering.

## Pré-requisitos
- Entender o frontend sintático do compilador.
- Saber diferenciar AST sintática de representação semântica.
- Conhecer o target atual em Lua 5.4.

## Tempo de leitura
12 a 15 minutos.

## Exemplo mínimo
```zt
state contador: int = 0
computed dobro: int = contador * 2
```

No fluxo interno:

```text
AST sintática
  -> binding de símbolos e tipos
  -> validação semântica
  -> lowering para forma reativa simplificada
```

## 1. O que a semântica resolve

Depois do parser, o compilador ainda não sabe:

- a que símbolo cada identificador se refere;
- qual é o tipo final de cada expressão;
- se um `return` é compatível com a função;
- se um `break` aparece em contexto válido;
- se `apply`, `redo` e `watch` fazem sentido naquele escopo.

A camada `semantic/` resolve exatamente isso.

## 2. Símbolos e escopos

Peças principais:

- `symbol`;
- `symbol_kind`;
- `variable_symbol`;
- `function_symbol`;
- `module_symbol`;
- `scope`.

Regras práticas de escopo:

```text
global const X  -> escopo global, imutável
global x        -> escopo global, mutável
const X         -> escopo local, imutável
x: int = 10     -> escopo local, mutável
state x         -> escopo reativo
```

O objetivo do binder é sair do modo “texto com forma” e chegar em “programa com nomes resolvidos”.

## 3. Tipos e relações

O sistema de tipos do Zenith precisa cobrir:

- tipos primitivos;
- coleções;
- unions;
- nullable;
- modificadores como `uniq`.

Mapa mental útil:

| Tipo Zenith | Semântica interna | Lua target |
|---|---|---|
| `int` | `IntType` | `number` |
| `float` | `FloatType` | `number` |
| `text` | `TextType` | `string` |
| `bool` | `BoolType` | `boolean` |
| `null` | `NullType` | `nil` |
| `list<T>` | `ListType(T)` | `table` |
| `map<K,V>` | `MapType(K,V)` | `table` |
| `grid<T>` | `GridType(T)` | `table` |
| `T?` | `UnionType(T, Null)` | `T | nil` |
| `A | B` | `UnionType(A, B)` | verificado em runtime |

## 4. Binding e análise semântica

Responsabilidades principais:

- `name_resolver`: ligar identificadores ao símbolo correto;
- `type_checker`: validar atribuições, chamadas e retornos;
- `control_flow`: detectar caminhos inválidos;
- `constant_folder`: avaliar constantes;
- `semantic_validator`: aplicar regras específicas do Zenith.

Invariantes da bound tree:

- todo `bound_expr` tem tipo resolvido;
- todo identificador aponta para um símbolo;
- nenhum nó bound carrega token cru do lexer.

## 5. Lowering: reduzir açúcar antes do backend

O lowering existe para que o backend Lua receba uma forma mais uniforme e previsível.

Transformações importantes:

| Construção Zenith | Forma lowered |
|---|---|
| `repeat 5 times` | loop equivalente a `for _i = 1, 5` |
| `"Olá, {name}"` | concatenação de partes |
| `state` / `computed` / `watch` | wrappers reativos |
| `attempt / rescue` | `pcall` wrapper |
| `expr!` | propagação explícita de erro |
| desestruturação | atribuições sequenciais |
| `apply Trait to Struct` | injeção de métodos no struct |

## 6. Regras de transpilação que precisam ficar previsíveis

### Variáveis e funções

- declarações devem preservar tipos na semântica, mas emitir Lua idiomático;
- funções assíncronas, quando existirem, precisam ser lowered antes do backend receber a forma final.

### Struct, trait e apply

O binder precisa validar:

- se o `struct` existe;
- se o `trait` existe;
- se os métodos obrigatórios foram satisfeitos;
- se o `redo` aponta para alvo válido.

O lowering ou backend então converte essa estrutura para o modelo de metatable em Lua.

### Enum com `match`

O frontend modela a forma do `match`. A semântica valida:

- compatibilidade entre padrão e expressão;
- existência dos campos;
- cobertura das possibilidades, quando aplicável.

Depois disso, o lowering pode transformar o `match` em condições mais simples para emissão.

### Reatividade

`state`, `computed` e `watch` não devem vazar complexidade para o backend. A ideia é:

- a semântica valida dependências;
- o lowering normaliza o modelo reativo;
- o runtime Lua só executa a infraestrutura já definida.

### Tratamento de erro

`attempt`, `rescue` e `!` precisam ser coerentes:

- `attempt/rescue` encapsula falha local;
- `!` propaga falha para o chamador.

Essa diferença semântica deve existir antes da emissão Lua.

## 7. Módulos e imports

A análise semântica também precisa lidar com:

- grafo de módulos;
- resolução de imports locais;
- builtins;
- aliases.

O princípio é que o código semântico entenda dependências, mas o backend continue sendo o único responsável por conhecer detalhes específicos do target.

## 8. Regra de ouro desta etapa

Se a dúvida for “isso já precisa conhecer Lua?”, a resposta normalmente é:

- **não**, se ainda estivermos resolvendo nomes, tipos ou regras da linguagem;
- **sim**, apenas quando a IR já estiver pronta e a questão for puramente de emissão.

## 9. Checklist para mudanças nesta camada

Ao adicionar uma construção que muda significado:

1. ampliar símbolos ou tipos, se necessário;
2. atualizar binder e validadores;
3. definir a forma lowered;
4. garantir diagnósticos claros;
5. só depois ajustar o backend Lua.

Essa ordem evita que decisões do target contaminem a semântica do Zenith.
