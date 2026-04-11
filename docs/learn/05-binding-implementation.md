# Módulo 05: Implementação do Binder (Projeto Zenith)

**Objetivo**: Analisar como o Zenith resolve nomes e gerencia a visibilidade usando o Binder.  
**Público-alvo**: Contribuidores e desenvolvedores que queiram entender a lógica semântica.  
**Contexto**: Trilha de Implementação (Código Real)

## Conteúdo Principal

A "inteligência" do Zenith reside no arquivo `src/semantic/binding/binder.lua`. Ele utiliza uma estrutura de dados de suporte chamada `Scope.lua` para gerenciar o que é visível e onde.

### 1. Escopos Aninhados (`Scope.lua`)
Todo escopo no Zenith tem um tipo (`GLOBAL`, `FUNCTION`, `BLOCK`, `STRUCT`) e uma referência para o escopo **pai**.
```lua
function Scope:lookup(name)
    local symbol = self.symbols[name]
    if symbol then return symbol end -- Achou aqui!
    if self.parent then 
        return self.parent:lookup(name) -- Procura no pai
    end
    return nil -- Símbolo não encontrado
end
```
Essa busca recursiva (subindo a árvore de escopos) é o que permite que uma função acesse variáveis globais, por exemplo.

### 2. A Estratégia Multi-pass
O Zenith não analisa o código em uma única rodada. Ele faz "viagens" pela AST (no método `Binder:bind`):

1. **Passo 1 (Coleta)**: Percorre o arquivo apenas para descobrir nomes de funções e structs. Isso permite que você chame uma função que foi escrita lá embaixo no arquivo.
2. **Passo 1.2 (Assinaturas)**: Processa os campos de structs e assinaturas de métodos de traits.
3. **Passo 2 (Verificação)**: Agora que todos os nomes já são conhecidos, o Binder mergulha nos corpos das funções para verificar se o código faz sentido.

### 3. Diagnósticos Semânticos
Quando o Binder falha em encontrar um símbolo, ele usa os diagnósticos:
```lua
if not symbol then
    self.diagnostics:report_error("ZT-S002", 
        string.format("símbolo '%s' não encontrado", node.name), 
        node.span)
end
```

## Decisões e Justificativas

- **Decisão**: Validação de contexto (ex: check de `break` fora de loops).
- **Motivo**: O Parser aceita `break` em qualquer lugar, mas o Binder verifica o escopo:
  ```lua
  function Scope:is_in_loop()
      if self.kind == Scope.Kind.LOOP then return true end
      return self.parent and self.parent:is_in_loop() or false
  end
  ```
  Se `is_in_loop()` retornar false e o Binder encontrar um `BREAK_STMT`, ele lança um erro.

## Exemplos de Código Real
Veja o arquivo [binder.lua](file:///C:/Users/raillen.DESKTOP-99RJ5M6/Documents/Projetos/zenith-lang-v2/src/semantic/binding/binder.lua). Procure pelo método `_declare`, que é o responsável pela Passagem 1 (colheita de símbolos).

## Impactos na Implementação
A separação entre `_declare` (registrar nome) e `_bind` (verificar corpo) resolve o problema de **dependências circulares**, tornando o Zenith uma linguagem muito mais flexível de escrever.

---

### 🏆 Desafio Zenith
Abra o arquivo `src/semantic/symbols/scope.lua`.
1. Localize a função `all_names`. Para que ela é usada? (Dica: Pense na experiência do usuário quando ele comete um erro de digitação).
2. Se você quisesse impedir o **Shadowing** (proibir declarar uma variável local com o mesmo nome de um parâmetro), onde você adicionaria essa verificação no `binder.lua`?

**Status**: Implementação Semântica Analisada
