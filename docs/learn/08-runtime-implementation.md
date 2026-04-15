# Modulo 08: Implementacao do Runtime

Objetivo: explicar o runtime real que sustenta a trilha ativa em src/backend/lua/runtime/zenith_rt.lua.

## 1. Papel do runtime

O runtime da trilha ativa nao serve apenas para reatividade. Ele tambem sustenta a surface real da linguagem em tres frentes:

- Optional e Outcome
- UFCS virtual
- indexacao segura de sequencias

## 2. Optional e Outcome

O runtime fornece o comportamento concreto para:

- Present e Empty
- Success e Failure
- unwrap
- unwrap_or
- is_present
- is_empty

Na pratica, isso significa:

- Success e tratado como presente
- Failure e tratado como vazio para fallback
- unwrap_or(default) funciona como base do fallback da trilha ativa

## 3. UFCS virtual

O binder e o codegen encaminham varios metodos para helpers do namespace zt no runtime.

Metodos efetivos hoje:

- len
- split
- push
- pop
- keys
- is_present
- is_empty
- unwrap
- to_text

Exemplos de chamadas que acabam no runtime:

~~~zt
var n = lista.len()
var partes = "a,b,c".split()
var txt = 42.to_text()
~~~

Observacao importante:

- split() sem argumento usa "," como padrao na trilha ativa via caminho UFCS

## 4. Indexacao segura

A filosofia vigente da trilha ativa e:

- listas e texto sao 1-based
- mapas usam a chave declarada
- acesso invalido em runtime gera erro Zenith proprio, nao erro cru do Lua

Helpers de runtime:

- zt.index_seq
- zt.index_text
- zt.index_any
- zt.slice

Codigo de diagnostico associado:

- ZT-R011 para indices ou slices fora dos limites

## 5. Reatividade continua existindo

A camada de reatividade permanece no runtime, mas ela nao e mais a unica historia importante. Para manutencao da linguagem estabilizada, Optional, Outcome, UFCS e indexacao segura sao partes obrigatorias da leitura do arquivo.

## 6. O que documentar ao tocar o runtime

Sempre espelhe nos docs quando o runtime mudar estas superficies:

- novos helpers UFCS
- semantica de fallback ou unwrap_or
- politica de indexacao
- codigos de erro runtime
