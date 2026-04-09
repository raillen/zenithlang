# Módulo 06: Implementação do Sistema de Tipos (Projeto Zenith)

**Objetivo**: Analisar como o Zenith representa e valida tipos internamente.  
**Público-alvo**: Contribuidores e desenvolvedores que queiram entender o motor de tipagem.  
**Contexto**: Trilha de Implementação (Código Real)

## Conteúdo Principal

Diferente do Lua, que trata tipos de forma fluida, o Zenith possui uma representação rigorosa chamada `ZenithType.lua`. Cada expressão na AST, após passar pelo Binder, recebe um desses objetos.

### 1. Categorias de Tipos (`ZenithType.Kind`)
O Zenith suporta diversos tipos de estruturas:
- `PRIMITIVE`: `int`, `float`, `text`, `bool`.
- `STRUCT / ENUM / TRAIT`: Tipos definidos pelo usuário.
* `NULLABLE`: O tipo `T?` que permite `null`.
* `UNION`: O tipo `A | B`.
* `GENERIC`: Coleções como `list<int>`.
* `FUNC`: Tipos de função (lambdas) como `(int) => text`.
* `TYPE_PARAM`: Placeholders para tipos genéricos em traits e structs.

### 2. O Algoritmo de Compatibilidade (`is_assignable_to`)
Esta é a função mais importante do sistema de tipos. Ela decide se o valor `A` pode ser guardado na variável `B`.

```lua
function ZenithType:is_assignable_to(other)
    if self == other then return true end -- Tipos idênticos

    -- Lógica de Traits: Uma Struct pode ser atribuída a uma Trait?
    if other.kind == ZenithType.Kind.TRAIT then
        if self.kind == ZenithType.Kind.STRUCT then
            return self:implements_trait(other) -- Verifica se a struct registrou esse apply
        end
    end
    -- ...
end
```

### 3. Variância em Funções (`Kind.FUNC`)
O Zenith implementa regras clássicas de variância para funções:
- **Retorno**: É **Covariante** (um `() => int` pode ser atribuído a um `() => any`).
- **Parâmetros**: É **Contravariante** (um `(any) => void` pode ser atribuído a um `(int) => void`).

Isso garante segurança matemática: você pode passar uma função que aceita "qualquer coisa" para alguém que prometeu passar apenas um "inteiro".

### 4. Implementação de Traits no Binder
Quando você escreve um bloco `apply`, o Binder faz o seguinte:
1. Localiza o símbolo da `Trait` e da `Struct`.
2. Verifica se todos os métodos exigidos na `Trait` estão presentes no `apply`.
3. Se tudo estiver correto, ele adiciona a `Trait` à lista de `implements` daquela `Struct`.

Isso torna a verificação de tipos ultra veloz, pois em tempo de execução o compilador só precisa checar se a Trait está na lista de implementações da Struct.

## Decisões e Justificativas

- **Decisão**: Tipagem nominal com exceções pragmáticas.
- **Motivo**: Embora o Zenith seja estrito, permitimos que um `int` seja atribuído a um `float` automaticamente. Isso reduz a carga cognitiva de precisar fazer casts manuais para operações matemáticas simples, mantendo o código limpo.

## Exemplos de Código Real
Veja o arquivo [zenith_type.lua](file:///C:/Users/raillen.DESKTOP-99RJ5M6/Documents/Projetos/zenith-lang-v2/src/semantic/types/zenith_type.lua). Note como a lógica de `is_assignable_to` cresce para lidar com Uniões e Genéricos, garantindo que `list<int>` não seja confundido com `list<text>`.

## Impactos na Implementação
Como o Zenith transpila para Lua (que não tem tipos), toda essa verificação de `ZenithType` serve para garantir que o código gerado nunca tente realizar operações impossíveis no runtime Lua, agindo como um "filtro de segurança".

---

### 🏆 Desafio Zenith
Abra o arquivo `src/semantic/types/zenith_type.lua`.
1. Localize a lógica de `NULLABLE`. Por que um tipo `NULL` (valor `null`) é atribuível a um `int?`, mas não a um `int` puro?
2. Como você alteraria a função `is_assignable_to` para permitir que todos os tipos da linguagem fossem convertidos automaticamente para `text` (string)? Isso seria bom ou ruim para a filosofia do Zenith?

**Status**: Implementação de Tipos Analisada
