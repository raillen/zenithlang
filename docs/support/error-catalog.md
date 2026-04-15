# Catalogo de Erros e Warnings

O compilador e o runtime atuais usam familias de codigo ZT-L, ZT-P, ZT-S, ZT-W e ZT-R.

## Lexico

### ZT-L001 - string nao terminada

Uma aspas foi aberta e nao foi fechada.

### ZT-L002 - numero seguido de identificador invalido

O lexer encontrou algo como 123abc.

### ZT-L003 - caractere desconhecido

Ha um simbolo que o lexer nao reconhece.

## Parsing

### ZT-P001 - forma sintatica incompleta ou token esperado

Aparece quando falta end, ), :, to ou outra peca estrutural.

Exemplos tipicos:

- esquecer end
- escrever apply Trait Player em vez de apply Trait to Player
- omitir separador estrutural esperado

### ZT-P002 - expressao esperada

Aparece quando o parser precisava de uma expressao e encontrou fim de linha, token errado ou operador sem operando.

## Semantica

### ZT-S001 - redefinicao

Mesmo simbolo declarado duas vezes no mesmo escopo, ou parametro generico repetido.

### ZT-S002 - simbolo, tipo ou membro inexistente

Pode indicar:

- tipo nao encontrado
- struct nao encontrada
- alias nao encontrado

### ZT-S008 - membro nao encontrado no tipo

Acesso a campo ou metodo inexistente.

### ZT-S009 - dependencia circular em alias

Dois aliases acabam apontando um para o outro.

### ZT-S100 - incompatibilidade de tipos

Caso mais comum do binder.

Exemplos:

- var x: int = "texto"
- var y: text = null
- chamada generica que nao satisfaz a restricao exigida

### ZT-S101 - declaracao ou operador invalido

Pode significar:

- variavel sem tipo obrigatorio
- operador usado com tipos invalidos
- declaracao sem padrao ou nome valido

### ZT-S102 - condicao, indice ou padrao com tipo invalido

Esse codigo e reutilizado para erros de contexto como:

- condicao de if ou while que nao e bool
- indice que nao e inteiro
- range com limites nao inteiros
- variante de padrao inexistente

### ZT-S103 - chamada invalida ou ? em tipo incompativel

Aparece quando algo nao chamavel e usado como funcao, ou quando ? e aplicado fora de Optional ou Outcome.

### ZT-S104 - contexto generico ou uso de ? invalido

Pode indicar:

- quantidade errada de argumentos genericos
- uso de ? fora de uma funcao compativel

### ZT-S105 - argumentos invalidos

Usado para:

- argumento nomeado desconhecido
- argumento faltante
- parametros duplicados
- incompatibilidade entre ? e o tipo de retorno da funcao atual

### ZT-S201 - self ou @campo fora de metodo

self e @nome so podem aparecer dentro de metodo de struct ou trait.

### ZT-S202 - await fora de funcao async

await so funciona dentro de async func ou lambda async.

### ZT-S301, ZT-S302, ZT-S303 - erros de trait e apply

- ZT-S301: trait nao encontrada
- ZT-S302: struct nao encontrada
- ZT-S303: struct nao implementa metodo exigido pela trait

## Warnings

### ZT-W001 - uso direto de null em tipo anulavel

O codigo usou null em um contexto T?. Ainda compila, mas a trilha ativa recomenda Optional ou Outcome.

### ZT-W002 - indice 0 em sequencia 1-based

Listas e texto sao 1-based na trilha ativa. Exemplos tipicos:

- lista[0]
- texto[0]
- 0..n em slice de sequencia

### ZT-W003 - atributo legado com @

Atributos de declaracao devem usar #[...]. A forma @atributo permanece apenas por compatibilidade temporaria.

## Runtime

### ZT-R011 - indice ou slice fora dos limites

Erro de runtime emitido pelos helpers de indexacao segura da trilha ativa.

Casos tipicos:

- lista[i] com i < 1 ou i > tamanho
- texto[i] com i fora do intervalo
- slice fora dos limites em lista ou texto

## Dicas praticas

- Em ZT-P001, revise o bloco logo acima da linha sinalizada.
- Em ZT-S100, compare tipo esperado e tipo real.
- Em ZT-W002, revise a indexacao pensando em base 1 para sequencias.
- Em ZT-W003, migre de @atributo para #[...].
- Em ZT-R011, verifique limites antes do acesso dinamico.
