# Módulo 06: Teoria de Tipos e Traits (Contratos)

**Objetivo**: Entender como o Zenith garante a segurança do código através de tipos e contratos de Traits.  
**Público-alvo**: Estudantes de ciência da computação e criadores de linguagens.  
**Contexto**: Trilha Teórica (Agnóstica)

## Conteúdo Principal

O sistema de tipos é a "Constituição" da sua linguagem. Ele define o que é permitido e o que é proibido.

### 1. Tipagem Estática vs Dinâmica
- **Tipagem Dinâmica** (ex: Lua, Python): "Atire primeiro, pergunte depois". O erro só acontece quando você executa o código.
- **Tipagem Estática** (ex: Zenith, Go, Rust): "Verifique antes de disparar". O compilador impede que o erro chegue ao usuário final.

### 2. O que é uma Trait?
Imagine que você está contratando um "Músico". Você não se importa se ele é um humano, um robô ou um alien, desde que ele saiba `tocar()` e `afinar()`.
Uma **Trait** é esse contrato. Ela define **o que** uma entidade deve ser capaz de fazer, sem se importar com **quem** ela é.

#### Diferencial do Zenith: Composição sobre Herança
Muitas linguagens usam "Herança" (Ex: `Cachorro herda de Animal`). No Zenith, focamos em **Composição** via Traits:
- Uma Struct `Cachorro` **aplica** a Trait `Barulhento`.
- Uma Struct `Alarme` também **aplica** a Trait `Barulhento`.

### 3. Verificação de Contrato
Quando você diz `apply Barulhento to Alarme`, o compilador faz um check-list:
- `Alarme` tem a função `fazer_barulho()`?
- A função tem os mesmos parâmetros e retorno exigidos pela Trait?
Se algum item falhar, o compilador trava a build. Isso é **Segurança de Tipo**.

## Decisões e Justificativas

- **Decisão**: Tipagem Nominal vs Estrutural.
- **Motivo**: No Zenith, usamos tipagem **Nominal** (o nome do tipo importa). Isso reduz a ambiguidade e ajuda na documentação e autocompletar da IDE, mantendo o baixo atrito cognitivo.

## Exemplos Comentados

### O Contrato
```text
TRAIT Curavel:
    func curar(pontos: int)
END

STRUCT Player:
    vida: int
END

APPLY Curavel TO Player:
    func curar(p) -- ERRO! Faltou o tipo do parâmetro 'p'
        self.vida += p
    end
END
```
O compilador Zenith é o "advogado" que garante que cada detalhe do contrato `Curavel` seja seguido à risca pela struct `Player`.

## Impactos na Implementação
O sistema de tipos é o que permite ao Zenith gerar código Lua altamente otimizado, pois sabemos exatamente o que cada variável contém antes mesmo de começar a rodar.

---

### 🏆 Desafio Zenith
1. Se uma `trait` exige um método `toString() -> text`, e minha `struct` já tem um método `toString() -> int`, o contrato é satisfeito? Por que o tipo de retorno é tão importante quanto o nome do método?
2. Como as Traits ajudam a manter o código "aberto para extensão, mas fechado para modificação"?

**Status**: Teoria de Tipos Concluída
