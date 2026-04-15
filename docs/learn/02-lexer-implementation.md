# Modulo 02: Implementacao do Lexer

Objetivo: explicar o lexer real da trilha ativa, com enfase no que existe hoje em src/syntax/lexer/lexer.lua e src/syntax/lexer/lexer.zt.

## Estrutura basica

O lexer mantem tres elementos centrais:

- texto fonte
- posicao atual
- saco de diagnosticos

Na implementacao atual, duas operacoes de leitura sao fundamentais:

~~~lua
function Lexer:current()
    if self.pos > #self.text then return "\0" end
    return self.text:sub(self.pos, self.pos)
end

function Lexer:peek(offset)
    local p = self.pos + (offset or 1)
    if p > #self.text then return "\0" end
    return self.text:sub(p, p)
end
~~~

Leitura pratica:

- current() devolve o caractere na posicao atual
- peek(offset) faz lookahead sem consumir entrada

## Por que current() e peek() coexistem

A trilha ativa usa os dois porque eles resolvem problemas diferentes:

- current() simplifica loops de consumo
- peek() resolve ambiguidade local, como float vs range e operadores compostos

Exemplo de leitura numerica:

~~~lua
while not self:is_at_end() and (is_digit(self:current()) or self:current() == ".") do
    if self:current() == "." then
        if self:peek() == "." then break end
        has_dot = true
    end
    self:advance()
end
~~~

## Interpolacao de string

A implementacao atual desmembra strings interpoladas em tokens menores, o que reduz a complexidade do parser.

Leitura conceitual:

- "ola {nome}" vira concatenacao de partes
- o parser enxerga expressoes normais, nao um tipo especial de string magica

## Tokens relevantes da trilha ativa

O lexer atual precisa reconhecer, entre outros:

- self
- where
- validate
- @ para self sugar
- # e [ para atributos #[...]
- ? e ! para fluxo seguro
- .. para range

## O que este modulo precisa refletir

Quando documentar ou alterar o lexer, preserve estas verdades da trilha ativa:

- current() continua sendo a leitura principal do cursor
- peek(offset = 1) continua sendo o mecanismo de lookahead
- validate e keyword real
- @campo e tokenizado no dominio de expressao, nao de atributo declarativo normativo
- #[...] e o caminho oficial de atributos de declaracao
