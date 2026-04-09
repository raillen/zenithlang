# Zenith Language — Documento de Implementação

> Versão de referência: v1.0-alpha  
> Target de saída: Lua 5.4  
> Paradigma: transpiler com frontend próprio

---

## Visão Geral

Zenith é uma linguagem de programação que transpila para Lua. Ela tem sintaxe própria, sistema de tipos explícito, reatividade embutida e tooling nativo. O compilador lê arquivos `.zt`, passa por análise semântica e gera código Lua válido.

O princípio central da arquitetura é:

> O frontend não sabe que Lua existe. Só o backend conhece Lua.


A missão do Zenith é:

> oferecer uma linguagem de programação legível, explícita, estável visualmente e cognitivamente acessível, sem abrir mão de recursos modernos úteis para scripting, automação, jogos, interfaces e ensino.

Essa missão implica que o Zenith não deve ser apenas:

- uma linguagem “mais bonita”;
- uma camada sintática sobre Lua;
- uma linguagem educacional simplista;
- uma linguagem minimalista demais para uso real.

O Zenith deve buscar equilíbrio entre:

- **clareza**;
- **expressividade**;
- **previsibilidade**;
- **baixo atrito cognitivo**;
- **facilidade de tooling**;
- **consistência arquitetural**.

Clareza acima de esperteza sintática

O Zenith deve preferir construções que:

- sejam rapidamente reconhecíveis;
- possuam significado visual claro;
- reduzam necessidade de inferência implícita;
- favoreçam leitura linear.

A linguagem não deve competir por ser a mais curta. Ela deve competir por ser a mais compreensível dentro do seu nicho.

A linguagem deve favorecer:

- blocos claramente delimitados;
- baixa oscilação visual entre estilos equivalentes;
- previsibilidade de abertura e fechamento;
- legibilidade em leitura contínua e leitura interrompida.


A linguagem deve favorecer:

- blocos claramente delimitados;
- baixa oscilação visual entre estilos equivalentes;
- previsibilidade de abertura e fechamento;
- legibilidade em leitura contínua e leitura interrompida.


Toda feature nova deve ser avaliada segundo a pergunta:

> o programador precisa lembrar regras demais ao usar isso?

O Zenith deve reduzir a necessidade de “segurar mentalmente” muitas exceções ao mesmo tempo.


A linguagem e o tooling devem ajudar o usuário a:

- interromper o trabalho e voltar depois;
- localizar rapidamente o ponto do fluxo;
- entender o que uma declaração faz sem reanalisar grandes trechos;
- identificar onde blocos começam e terminam.

Esse ponto é especialmente relevante para TDAH.


A linguagem deve favorecer:

- palavras-chave reconhecíveis;
- baixa densidade simbólica desnecessária;
- boa separação espacial;
- baixa confusão entre formas parecidas.

Esse ponto é especialmente relevante para dislexia.

Não basta a sintaxe ser boa. Também é necessário que:

- os erros reflitam a filosofia da linguagem;
- o formatter preserve a proposta visual;
- o linter reforce boas práticas compatíveis com acessibilidade;
- o LSP reduza perda de contexto;
- a documentação ensine por progressão cognitiva, não apenas por completude técnica.


A reimplementação do Zenith deve assumir o seguinte posicionamento:

Zenith é uma linguagem de alto nível, estaticamente analisada, transpilada para Lua.

A linguagem deve ser boa principalmente para:

- ensino progressivo de programação;
- scripting local;
- automação;
- prototipagem de lógica;
- jogos e ferramentas apoiadas em ecossistemas Lua;
- aplicações com estado reativo;
- pequenos e médios projetos onde legibilidade importa mais que microconcisão.

Pode crescer para:

- UI declarativa;
- apps orientados a estado;
- bibliotecas de domínio específico;
- ferramentas educacionais.


Zenith não deve perseguir, no curto prazo:

- metaprogramação extremamente livre;
- sintaxe excessivamente simbólica;
- features avançadas difíceis de explicar pedagogicamente;
- múltiplos paradigmas concorrentes sem organização clara.

---

## Princípio de Separação de Camadas

O compilador Zenith é dividido em três representações internas, completamente separadas:

**1. Syntax Tree** — o que o usuário escreveu  
**2. Bound Tree** — o que o código significa (tipos, símbolos, escopos)  
**3. Lua IR / Lua AST** — o que será emitido em Lua  

Misturar essas camadas é o erro mais comum em transpilers. Um nó semântico nunca deve "saber" como será emitido. Um emissor nunca deve fazer resolução de nomes.

---

## Estrutura de Pastas

```text
zenith/
│
├── docs/
│   ├── language-spec/
│   │   ├── syntax.md
│   │   ├── semantics.md
│   │   ├── types.md
│   │   └── transpilation-rules.md
│   ├── roadmap/
│   │   ├── v0.1.md
│   │   ├── v0.2.md
│   │   └── v1.0.md
│   └── decisions/
│       ├── adr-001-ast-shape.md
│       ├── adr-002-type-system.md
│       └── adr-003-lua-target.md
│
├── examples/
│   ├── hello_world.zt
│   ├── variables.zt
│   ├── functions.zt
│   ├── structs.zt
│   ├── traits.zt
│   ├── reactivity.zt
│   └── modules.zt
│
├── testdata/
│   ├── lexer/
│   ├── parser/
│   ├── semantic/
│   ├── transpiler/
│   └── integration/
│
├── src/
│   ├── cli/
│   ├── common/
│   ├── source/
│   ├── diagnostics/
│   ├── syntax/
│   ├── semantic/
│   ├── lowering/
│   ├── backend/
│   │   └── lua/
│   ├── project_system/
│   └── pipeline/
│
├── tests/
│   ├── lexer_tests/
│   ├── parser_tests/
│   ├── semantic_tests/
│   ├── lowering_tests/
│   ├── lua_backend_tests/
│   ├── snapshot_tests/
│   └── integration_tests/
│
└── tools/
    ├── scripts/
    ├── grammar-tools/
    └── snapshot-updater/
```

---

## Detalhamento de Cada Camada

### 1. `source/` — Representação do Arquivo de Origem

Responsável por representar o texto bruto do arquivo antes de qualquer análise.

| Arquivo | Responsabilidade |
|---|---|
| `source_file` | Representa um arquivo `.zt` físico ou lógico |
| `source_text` | Acesso ao texto bruto, caracteres e slices |
| `span` | Intervalo de texto com `start`, `length` e `end` |
| `location` | Linha e coluna derivadas de um `span` |
| `line_map` | Mapeia offsets de byte para linha/coluna |

Invariante: nenhum outro módulo acessa texto-fonte diretamente. Tudo passa por `source_text` e `span`.

---

### 2. `diagnostics/` — Erros, Avisos e Dicas

Centraliza todos os diagnósticos do compilador. Toda fase produz diagnósticos através desta camada.

| Arquivo | Responsabilidade |
|---|---|
| `diagnostic` | Estrutura de erro/aviso/dica com span e mensagem |
| `diagnostic_bag` | Acumula diagnósticos de todas as fases |
| `diagnostic_code` | Enum com todos os códigos fixos de erro |
| `diagnostic_reporter` | Converte diagnósticos para saída legível |
| `diagnostic_formatting` | Formata trechos de código com contexto visual |

Saída esperada (compatível com a spec da linguagem):

```
❌ Erro [ZT-001]: variável 'x' não declarada
  --> src/main.zt:12:5
   |
12 |     x = 10
   |     ^ símbolo não encontrado
💡 Dica: você quis dizer 'count'?
```

Invariante: diagnósticos devem sempre ter span preciso. Mensagens vagas como "erro de sintaxe" são proibidas.

---

### 3. `syntax/` — Forma do Código

Toda análise de forma (sem significado) acontece aqui. Esta camada não sabe nada sobre tipos, escopos ou Lua.

#### 3.1 `syntax/tokens/`

| Arquivo | Responsabilidade |
|---|---|
| `token_kind` | Enum de todos os tipos de token (palavras-chave, operadores, literais) |
| `token` | Estrutura do token: tipo, lexema, valor, span |
| `keyword_table` | Mapa de strings para `token_kind` (ex: `"func"` → `Func`) |
| `operator_table` | Operadores, precedência e associatividade |

Palavras-chave Zenith mapeadas:

const, redo, global, state, computed, watch, namespace
pub, struct, enum, trait, apply, func, return
async, await, if, elif, else, match, case
for, in, while, repeat, times, break, continue
and, or, not, where, grid, uniq
self, it, _, import, export, as, to
attempt, rescue, true, false, null

#### 3.2 `syntax/lexer/`

| Arquivo | Responsabilidade |
|---|---|
| `lexer` | Tokeniza o texto-fonte |
| `lexer_state` | Posição atual, linha, coluna |
| `lexer_reader` | Leitura controlada de caracteres com lookahead |
| `lexer_rules` | Regras para literais, strings, comentários, identificadores |

Regras de tokenização relevantes para Zenith:
- Comentários: `--` (linha única, herança Lua)
- Strings: `"texto"` com interpolação `{expr}`
- Ranges: `0..5`
- Nullable shorthand: `text?`
- Union types: `int | text`
- Bang operator: `expr!`

#### 3.3 `syntax/ast/`

| Arquivo | Responsabilidade |
|---|---|
| `syntax_node` | Tipo-base de todos os nós sintáticos |
| `syntax_kind` | Enum de todos os tipos de nó |
| `expr_syntax` | Nós de expressão (binária, chamada, literal, etc.) |
| `stmt_syntax` | Nós de statement (if, while, for, repeat, etc.) |
| `decl_syntax` | Nós de declaração (struct, trait, func, enum, @attr, etc.) |
| `type_syntax` | Nós de tipo (primitivo, union, nullable, genérico, modificado) |
| `compilation_unit_syntax` | Nó raiz de um arquivo parseado |

Nós de expressão para Zenith:

```
LiteralExpr       -- 10, "texto", true, null
BinaryExpr        -- a + b, a == b, a and b
UnaryExpr         -- not x
IdentifierExpr    -- nome de variável
CallExpr          -- func(args)
MemberExpr        -- obj.campo
IndexExpr         -- list[i]
RangeExpr         -- 0..5
MatchExpr         -- match value ... (se usado como expressão)
MatchStmt         -- match value: case ... else ... (formato statement)
InterpolationExpr -- "Olá, {name}!" (desugared para concatenação no lexer/parser)
LambdaExpr        -- it > 5, it * 2
BangExpr          -- expr!
```

Nós de statement:

```
AssignStmt        -- x = valor
IfStmt            -- if / elif / else / end
WhileStmt         -- while / end
ForInStmt         -- for item in list / end
RepeatTimesStmt   -- repeat N times / end
BreakStmt
ContinueStmt
ReturnStmt
WatchStmt         -- watch state: ... end
MatchStmt         -- match expr: case ... else ... end
AttemptStmt       -- attempt / rescue e / end
```

Nós de declaração:

```
VarDecl           -- x: int = 10
ConstDecl         -- const PI: float = 3.14
GlobalDecl        -- global x: int = 10
StateDecl         -- state counter: int = 0
ComputedDecl      -- computed doubled: int = counter * 2
FuncDecl          -- func name(params) -> type ... end
AsyncFuncDecl     -- async func ...
StructDecl        -- struct Name ... end
EnumDecl          -- enum Name ... end
TraitDecl         -- trait Name ... end
ApplyDecl         -- apply Trait to Struct
ImportDecl        -- import "path" / import std.time as alias
ExportDecl        -- export func ...
RedoDecl          -- redo func Type.name(...)
NamespaceDecl     -- namespace name.subname
AttributeNode     -- @name(args)
```

#### 3.4 `syntax/parser/`

| Arquivo | Responsabilidade |
|---|---|
| `parser` | Ponto central, coordena o parsing |
| `parser_context` | Estado compartilhado: tokens, posição, diagnósticos |
| `parse_expressions` | Parsing de expressões com precedência (Pratt parser recomendado) |
| `parse_statements` | Parsing de statements |
| `parse_declarations` | Parsing de declarações |
| `parse_types` | Parsing de tipos (union, nullable, genérico) |
| `parse_helpers` | Consumo de tokens, lookahead, recuperação de erros |

Invariante: o parser nunca resolve nomes, nunca verifica tipos, nunca emite Lua. Ele apenas produz uma `compilation_unit_syntax` válida ou com erros marcados.

Estratégia de recuperação de erro: ao encontrar token inesperado, avança até o próximo ponto de sincronização (`end`, `func`, `struct`, nova linha com declaração).

---

### 4. `semantic/` — Significado do Código

Toda análise de significado acontece aqui. Esta camada não sabe nada sobre Lua.

#### 4.1 `semantic/symbols/`

| Arquivo | Responsabilidade |
|---|---|
| `symbol` | Tipo-base de símbolo |
| `symbol_kind` | Enum: variável, constante, função, struct, trait, enum, módulo |
| `variable_symbol` | Símbolo de variável com tipo, escopo e mutabilidade |
| `function_symbol` | Símbolo de função com assinatura completa |
| `module_symbol` | Símbolo de módulo ou import |
| `scope` | Escopo léxico com tabela de símbolos e escopo pai |

Regras de escopo Zenith:

```
global const X   → escopo global, imutável
global x         → escopo global, mutável
const X          → escopo local, imutável
x: int = 10      → escopo local, mutável
state x          → escopo reativo
```

#### 4.2 `semantic/types/`

| Arquivo | Responsabilidade |
|---|---|
| `type_symbol` | Tipo-base do sistema de tipos |
| `builtin_types` | int, float, text, bool, null, list, map, grid, set |
| `type_relations` | Compatibilidade e subtipagem |
| `type_conversions` | Conversões explícitas via `to_int`, `to_text`, etc. |

Tipos Zenith e representação interna:

| Tipo Zenith | Representação Semântica | Lua target |
|---|---|---|
| `int` | `IntType` | `number` (inteiro) |
| `float` | `FloatType` | `number` (float) |
| `text` | `TextType` | `string` |
| `bool` | `BoolType` | `boolean` |
| `null` | `NullType` | `nil` |
| `list<T>` | `ListType(T)` | `table` (array) |
| `map<K,V>` | `MapType(K,V)` | `table` (hash) |
| `grid<T>` | `GridType(T)` | `table` (2D array) |
| `set<T>` | `SetType(T)` | `table` (hash set) |
| `uniq C` | `ModifiedType(uniq, C)` | `table` (coleção sem duplicatas) |
| `uniq list<T>` | `ModifiedType(List, Uniq)` | `table` (set-like) |
| `T?` | `UnionType(T, Null)` | `T | nil` |
| `A | B` | `UnionType(A, B)` | verificado em runtime |

#### 4.3 `semantic/binding/`

| Arquivo | Responsabilidade |
|---|---|
| `binder` | Liga nós sintáticos ao modelo semântico |
| `bound_node` | Tipo-base da bound tree |
| `bound_expr` | Expressões com tipo resolvido |
| `bound_stmt` | Statements com símbolos resolvidos |
| `bound_decl` | Declarações com símbolo registrado |
| `binding_context` | Escopo ativo, símbolos, estado do binder |

Invariante do bound tree:
- Todo `bound_expr` tem tipo resolvido. Nunca `null`.
- Todo identificador tem símbolo associado. Nunca pendente.
- Nenhum nó bound contém token cru do lexer.

#### 4.4 `semantic/analysis/`

| Arquivo | Responsabilidade |
|---|---|
| `name_resolver` | Resolve identificadores para símbolos no escopo correto |
| `type_checker` | Valida compatibilidade de tipos em atribuições, retornos e chamadas |
| `control_flow` | Detecta caminhos sem `return`, `break` fora de loop, etc. |
| `constant_folder` | Avalia expressões constantes em tempo de compilação |
| `semantic_validator` | Regras extras: `pub` fora de struct, `apply` para trait inexistente, etc. |

#### 4.5 `semantic/compilation/`

| Arquivo | Responsabilidade |
|---|---|
| `compilation` | Coordena a análise semântica de uma unidade |
| `module_graph` | Grafo de dependências entre módulos |
| `semantic_model` | Exposição estruturada do resultado semântico para o backend |

---

### 5. `lowering/` — Simplificação Antes da Emissão

Transforma construções complexas em formas mais simples antes de gerar Lua. O backend Lua recebe uma IR normalizada, sem açúcar sintático.

| Arquivo | Responsabilidade |
|---|---|
| `lowered_node` | Tipo-base da IR simplificada |
| `lowered_expr` | Expressões reduzidas |
| `lowered_stmt` | Statements reduzidos |
| `lowerer` | Executa as transformações |
| `lowering_rules` | Regras de desugaring por construção |

Transformações de lowering para Zenith:

| Construção Zenith | Forma lowered |
|---|---|
| `repeat 5 times` | `for _i = 1, 5 do` (em Lua) |
| `for index, item in list` | iteração com índice explícito |
| `"Olá, {name}!"` | concatenação de partes |
| `x: T? = null` | union com null desmembrada |
| `apply Trait to Struct` | injeção de métodos no struct |
| `state x` / `computed y` | wrapper reativo |
| `watch x: ... end` | observer registrado |
| `attempt / rescue` | pcall wrapper |
| `expr!` | if error then propagate end |
| `redo func T.name(...)` | substituição de método no prototype |
| Desestruturação `[a, b] = list` | atribuições sequenciais |

---

### 6. `backend/lua/` — Geração de Código Lua

Única camada que conhece Lua. Recebe a IR lowered e produz texto Lua válido.

#### 6.1 Informações do target

| Arquivo | Responsabilidade |
|---|---|
| `lua_target_info` | Versão Lua alvo (5.4), recursos disponíveis, limitações |
| `lua_reserved_words` | Todas as palavras reservadas Lua: `and`, `break`, `do`, `else`, `elseif`, `end`, `false`, `for`, `function`, `goto`, `if`, `in`, `local`, `nil`, `not`, `or`, `repeat`, `return`, `then`, `true`, `until`, `while` |
| `lua_name_mangler` | Renomeia identificadores que conflitam com palavras reservadas Lua |

Regra do name mangler: se um identificador Zenith for palavra reservada em Lua, adiciona prefixo `_zt_`. Exemplo: se o usuário criar variável `end`, ela vira `_zt_end` no output.

#### 6.2 Lua AST

| Arquivo | Responsabilidade |
|---|---|
| `lua_node` | Tipo-base da AST Lua |
| `lua_expr` | Expressões Lua |
| `lua_stmt` | Statements Lua |
| `lua_chunk` | Unidade raiz Lua (bloco de código completo) |

#### 6.3 Emissores

| Arquivo | Responsabilidade |
|---|---|
| `lua_emitter` | Coordena a geração Lua a partir da IR lowered |
| `emit_expressions` | Emite expressões Lua |
| `emit_statements` | Emite statements Lua |
| `emit_declarations` | Emite declarações (local, function, etc.) |
| `emit_modules` | Emite imports/exports/require |
| `emit_runtime_helpers` | Injeta helpers do runtime no output |

Invariante do emitter:
- Nunca emitir identificador Lua inválido.
- Toda saída deve ser sintaticamente válida em Lua 5.4.
- Nunca produzir globals implícitas (usar `local` sempre).

#### 6.4 Formatação

| Arquivo | Responsabilidade |
|---|---|
| `lua_writer` | Escrita textual final do código Lua |
| `indentation_writer` | Controle de indentação e layout |
| `lua_pretty_printer` | Formatação legível do Lua gerado |

#### 6.5 Runtime

| Arquivo | Responsabilidade |
|---|---|
| `prelude.lua` | Helpers base sempre injetados no output |
| `std_array.lua` | Helpers para `list<T>`: append, remove, contains, size |
| `std_string.lua` | Helpers para `text`: len, upper, lower, trim, split |
| `std_object.lua` | Helpers para structs e traits: apply, self binding |
| `runtime_registry` | Decide quais helpers incluir com base no que foi usado |

O `runtime_registry` analisa o output do lowerer e injeta apenas os helpers necessários. Isso evita gerar código Lua inchado.

---

## Regras de Transpilação

Esta seção documenta como cada construção Zenith vira Lua.

### Variáveis

```zt
x: int = 10
const PI: float = 3.14159
global config: map = {}
```

```lua
local x = 10
local PI = 3.14159
_G.config = {}
```

### Funções

```zt
func greet(name: text) -> text
    return "Olá, " + name
end
```

```lua
local function greet(name)
    return "Olá, " .. name
end
```

### Struct

```zt
struct Player
    pub name: text
    health: int = 100
end

p: Player = Player { name: "Zenith" }
```

```lua
local Player = {}
Player.__index = Player

function Player.new(fields)
    local self = setmetatable({}, Player)
    -- Validação manual (emissão básica do 'where' e atributos)
    if fields.health and not (fields.health >= 0) then error("contract violation: health where it >= 0") end
    self.name = fields.name
    self.health = fields.health or 100
    return self
end

local p = Player.new({ name = "Zenith" })
```

### Trait e Apply

```zt
trait Flyable
    pub func fly()
        print(.name + " voa!")
    end
end

apply Flyable to Dragon
```

```lua
local function Flyable_fly(self)
    print(self.name .. " voa!")
end

function Dragon:fly()
    return Flyable_fly(self)
end
```

### Enum com match

```zt
enum Direction
    Up
    Down
end

match dir:
    case Up: print("subindo")
    case Down: print("descendo")
    else: print("outro")
end
```

```lua
local Direction = { Up = "Up", Down = "Down" }

local _match = dir
if _match == Direction.Up then
    print("subindo")
elseif _match == Direction.Down then
    print("descendo")
else
    print("outro")
end
```

### Reatividade (state / computed / watch)

Reatividade é emulada via tabela observável no runtime:

```zt
state counter: int = 0
computed doubled: int = counter * 2

watch counter:
    print("Mudou para: {counter}")
end
```

```lua
local _state = _zt_reactive_new()

_zt_state_set(_state, "counter", 0)
_zt_computed(_state, "doubled", function()
    return _zt_state_get(_state, "counter") * 2
end)
_zt_watch(_state, "counter", function()
    print("Mudou para: " .. tostring(_zt_state_get(_state, "counter")))
end)
```

O helper `_zt_reactive_new` vem do `prelude.lua`.

### Attempt / Rescue

```zt
attempt
    fail("erro")
rescue e
    print(e.message)
end
```

```lua
local _ok, _err = pcall(function()
    error({ message = "erro", code = 0 })
end)
if not _ok then
    local e = _err
    print(e.message)
end
```

### Bang Operator

```zt
result = dangerous()!
```

```lua
local _res, _err = dangerous()
if _err ~= nil then return nil, _err end
local result = _res
```

### Módulos

```zt
-- Arquivo: math_utils.zt
export func double(x: int) -> int
    return x * 2
end
```

```lua
-- math_utils.lua
local M = {}

function M.double(x)
    return x * 2
end

return M
```

```zt
-- Uso:
import "math_utils"
math_utils.double(5)
```

```lua
local math_utils = require("math_utils")
math_utils.double(5)
```

### Import de builtins

```zt
import std.time
```

```lua
local _zt_time = require("zenith.std.time")

-- Híbrido e Alias
-- import std.math as m -> local m = require("zenith.std.math")
-- import "myfile" as My -> local My = require("myfile")
```

---

## Pipeline de Compilação

A ordem das fases:

```
arquivo .zt
    ↓
[source] — carrega texto e cria line_map
    ↓
[lexer] — produz lista de tokens
    ↓
[parser] — produz compilation_unit_syntax (AST sintática)
    ↓
[binder] — resolve nomes e produz bound tree
    ↓
[type checker] — valida tipos
    ↓
[semantic validator] — regras extras
    ↓
[lowerer] — simplifica construções em IR
    ↓
[lua emitter] — gera Lua AST a partir da IR
    ↓
[lua writer] — serializa Lua AST em texto
    ↓
arquivo .lua
```

Cada fase retorna um `stage_result` com:
- output da fase (quando há)
- diagnósticos acumulados
- flag de sucesso

Se alguma fase produz erros críticos, o pipeline para antes de continuar.

### Arquivos do pipeline

| Arquivo | Responsabilidade |
|---|---|
| `compile_pipeline` | Orquestra todas as fases em ordem |
| `pipeline_context` | Contexto compartilhado entre fases |
| `pipeline_result` | Resultado global da compilação |
| `stage_result` | Resultado por fase individual |

---

## Sistema de Projeto

| Arquivo | Responsabilidade |
|---|---|
| `project_file` | Modelo do arquivo `zenith.toml` ou `zenith.json` |
| `project_loader` | Carrega e valida o arquivo de projeto |
| `module_resolver` | Resolve módulos da std e builtins |
| `import_resolver` | Resolve imports locais para caminhos de arquivo |
| `build_plan` | Define a ordem de compilação baseada no `module_graph` |

Exemplo de arquivo de projeto:

```toml
[project]
name = "minha-app"
version = "0.1.0"
entry = "src/main.zt"
target = "lua54"

[output]
dir = "dist/"
runtime = "bundle"  -- bundle | separate | none
```

---

## CLI

| Comando | O que faz |
|---|---|
| `zt build` | Compila o projeto completo |
| `zt run` | Compila e executa com lua |
| `zt check` | Valida sem emitir Lua |
| `zt transpile [file]` | Transpila um arquivo específico |
| `zt test` | Detecta e executa arquivos `*_test.zt` |

| Arquivo | Responsabilidade |
|---|---|
| `main` | Ponto de entrada, lê args e chama dispatcher |
| `command_dispatcher` | Encaminha para o comando correto |
| `command_build` | Lógica do `zt build` |
| `command_run` | Lógica do `zt run` |
| `command_check` | Lógica do `zt check` |
| `command_transpile` | Lógica do `zt transpile` |

---

## Testes

### Estrutura por fase

Cada fase tem sua própria pasta de testes isolada. Nunca testar tudo apenas por integração.

```
tests/
├── lexer_tests/        -- dado texto, espero estes tokens
├── parser_tests/       -- dado texto, espero esta AST
├── semantic_tests/     -- dado AST, espero estes erros ou símbolos
├── lowering_tests/     -- dado bound tree, espero esta IR
├── lua_backend_tests/  -- dado IR, espero este texto Lua
├── snapshot_tests/     -- dado arquivo .zt, espero este arquivo .lua
└── integration_tests/  -- dado projeto completo, espero saída correta
```

### Snapshot tests

Para o backend Lua, use snapshot tests. Armazene o `.zt` e o `.lua` esperado. Quando o output mudar, o teste falha e você decide se é regressão ou mudança intencional.

Exemplo de par de snapshot:

`testdata/transpiler/struct_basic.zt`:
```zt
struct Point
    pub x: int
    pub y: int
end

p: Point = Point { x: 1, y: 2 }
```

`testdata/transpiler/struct_basic.lua` (esperado):
```lua
local Point = {}
Point.__index = Point

function Point.new(fields)
    local self = setmetatable({}, Point)
    self.x = fields.x
    self.y = fields.y
    return self
end

local p = Point.new({ x = 1, y = 2 })
```

### Framework de testes nativo

Arquivos `*_test.zt` são detectados automaticamente pelo `zt test`:

```zt
describe("Funções matemáticas", func()
    it("deve dobrar um número", func()
        val: int = double(5)
        expect(val).to_equal(10)
    end)
end)
```

---

## Estrutura Mínima para v0.1

Para começar pequeno e funcional:

```text
src/
  cli/
    main
  source/
    source_text
    span
  diagnostics/
    diagnostic
    diagnostic_bag
  syntax/
    tokens/
      token_kind
      token
      keyword_table
    lexer/
      lexer
    ast/
      syntax_node
      expr_syntax
      stmt_syntax
      decl_syntax
    parser/
      parser
  semantic/
    symbols/
      symbol
      scope
    binding/
      binder
    analysis/
      type_checker
  lowering/
    lowerer
  backend/
    lua/
      lua_emitter
      lua_writer
      runtime/
        prelude.lua
  pipeline/
    compile_pipeline
```

Com essa estrutura mínima é possível suportar:
- declaração de variáveis com tipo
- expressões aritméticas e booleanas
- funções simples com retorno
- `if` / `elif` / `else`
- `while`
- transpile básico para Lua válido

---

## Convenções de Código

### Nomenclatura

| Contexto | Convenção | Exemplo |
|---|---|---|
| Pastas | snake_case | `syntax/tokens/` |
| Arquivos | snake_case | `parse_expressions` |
| Tipos / structs | PascalCase | `SyntaxNode`, `BoundExpr` |
| Funções | snake_case | `parse_function_decl` |
| Variáveis | snake_case | `token_kind` |
| Constantes | UPPER_SNAKE | `MAX_ERRORS` |

### Sufixos por camada

| Sufixo | Onde usar |
|---|---|
| `*Syntax` | AST sintática |
| `*Symbol` | Símbolos semânticos |
| `*Type` | Tipos semânticos |
| `*Bound` / `Bound*` | Bound tree |
| `*Lowered` / `Lowered*` | IR lowered |
| `*Lua` / `Lua*` | Backend Lua |
| `*Emitter` | Emissores |
| `*Context` | Contextos de fase |
| `*Result` | Resultados de fase |
| `*Resolver` | Resolvedores de nome/módulo |

### Tamanho de arquivo

| Tipo | Faixa saudável |
|---|---|
| Utilitário simples | 50–150 linhas |
| Estruturas centrais | 100–250 linhas |
| Parser parcial | 150–350 linhas |
| Emissor parcial | 150–350 linhas |
| Tabelas e definições | 200–500 linhas |

Se um arquivo passar muito disso, pergunte: ele tem mais de uma responsabilidade? Se sim, quebre.

---

## Ordem de Implementação Recomendada

### Etapa 1 — Base léxica
1. `source_text`, `span`, `location`, `line_map`
2. `diagnostic`, `diagnostic_bag`
3. `token_kind`, `token`, `keyword_table`, `operator_table`
4. `lexer`

### Etapa 2 — Parsing
5. `syntax_node`, `syntax_kind`
6. `expr_syntax`, `stmt_syntax`, `decl_syntax`, `type_syntax`
7. `parser` com suporte a variáveis, expressões, `if`, `while`, funções

### Etapa 3 — Semântica
8. `symbol`, `scope`
9. `binder`
10. `type_checker` básico (tipos primitivos)

### Etapa 4 — Transpile Lua
11. `lowerer` básico (desugaring mínimo)
12. `lua_emitter`, `lua_writer`
13. `prelude.lua`, `runtime_registry`

### Etapa 5 — Tooling
14. `compile_pipeline`
15. CLI com `build`, `check`, `transpile`
16. Snapshot tests
17. `project_file`, `module_resolver`

### Etapa 6 — Funcionalidades avançadas
18. `struct`, `trait`, `apply`
19. `enum` + `match`
20. `state`, `computed`, `watch`
21. `async` / `await`
22. `attempt` / `rescue` + bang operator
23. Sistema de módulos completo
24. Framework de testes nativo (`zt test`)

---

## Documentos Internos Recomendados

### `docs/language-spec/syntax.md`
- Gramática formal (BNF ou EBNF)
- Precedência de operadores
- Regras de bloco e `end`
- Casos especiais do lexer

### `docs/language-spec/semantics.md`
- Regras de escopo por declaração
- Resolução de nomes
- Regras de mutabilidade
- Controle de fluxo e retorno obrigatório

### `docs/language-spec/types.md`
- Tipos primitivos e compostos
- Union types e nullable
- Regras de compatibilidade
- Conversões explícitas disponíveis

### `docs/language-spec/transpilation-rules.md`
- Mapeamento de cada construção para Lua
- Helpers de runtime exigidos por construção
- Limitações do target Lua
- Decisões de representação (ex: struct como metatable)

### `docs/decisions/` (ADRs)
Registre decisões importantes com contexto:
- `adr-001`: por que usar metatable para struct
- `adr-002`: por que reatividade usa observer table
- `adr-003`: por que Lua 5.4 como target mínimo

---

*Zenith Language — Documento de Implementação v1.0-alpha*
