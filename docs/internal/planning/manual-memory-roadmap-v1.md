# Zenith Manual Memory Roadmap 1.0

> Roadmap exploratorio para gerenciamento de memoria manual opcional.
> Status: proposta futura, ainda a decidir.
> Criado: 2026-04-26.

## Objetivo

Este documento registra uma possibilidade futura para a linguagem Zenith:
permitir controle manual de memoria sem tornar esse controle obrigatorio.

O modelo padrao da linguagem continua sendo automatico.
Codigo comum nao deve precisar pensar em alocador, ponteiro cru ou liberacao
manual.

## Contexto da conversa

Esta proposta nasceu de uma pergunta sobre o custo tecnico de implementar
controle de memoria manual opcional na Zenith.

A conversa partiu de quatro pontos:

1. A Zenith ja tem um runtime com ARC para valores heap-managed.
2. O controle manual nao deve virar obrigacao para codigo comum.
3. Borealis, FFI, compilador, parser e buffers binarios podem se beneficiar de
   controle mais previsivel.
4. A linguagem deve continuar legivel e acessivel para pessoas com TDAH e
   dislexia.

Durante a conversa, foram comparados quatro caminhos:

- modelo C-like, com `malloc` e `free` expostos diretamente;
- modelo Zig/Nim, com allocators explicitos, arenas e ferramentas de debug;
- modelo Rust-like, com ownership, borrow checker, move e lifetimes;
- modelo hibrido, mantendo ARC como padrao e expondo memoria manual apenas em
  APIs explicitas.

O caminho C-like foi considerado simples de implementar no inicio, mas ruim
para a linguagem como experiencia principal. Ele tende a espalhar ponteiros,
`free` manual e bugs dificeis de diagnosticar.

O caminho Zig/Nim foi considerado mais alinhado com a Zenith porque permite
controle explicito sem obrigar todos os usuarios a pensar em ponteiros. Arenas,
allocators e debug allocator criam um vocabulario mais legivel do que
`malloc/free` soltos.

O caminho Rust-like foi considerado tecnicamente forte, mas caro demais para
ser o primeiro passo. Ownership, borrow checker e lifetimes mudariam o coracao
da linguagem e poderiam tornar a Zenith menos acessivel se entrassem cedo
demais.

O caminho hibrido ficou como melhor hipotese:

- ARC continua sendo o padrao;
- `defer` e destrutores melhoram gerenciamento de recursos;
- `std.mem.Allocator` permite controle manual explicito;
- arenas cobrem muitos casos reais sem exigir `free` item por item;
- `DebugAllocator` ajuda a manter diagnosticos claros;
- `std.unsafe` concentra ponteiros crus e FFI avancado;
- `owned<T>` e `borrow<T>` ficam como possibilidade futura, nao como MVP.

## Por que este caminho pode ser bom

Ele pode ser bom para a Zenith porque preserva a simplicidade do codigo comum e
abre uma porta para dominios mais exigentes.

Beneficios esperados:

- jogos e engine podem reduzir alocacoes por frame;
- parser e compilador podem usar arenas por fase;
- FFI fica mais honesto ao lidar com buffers e ponteiros nativos;
- hot paths podem evitar custo excessivo de ARC;
- runtime e stdlib ganham um vocabulario mais preciso para ownership externo;
- usuarios avancados ganham controle sem mudar a experiencia dos iniciantes.

Esse caminho tambem combina com a filosofia da Zenith se a regra for clara:

```text
sem std.mem = automatico
com std.mem = controle explicito
com std.unsafe = baixo nivel e responsabilidade maior
```

## Por que este caminho pode ser ruim

Ele pode ser ruim se a feature crescer sem limites.

Riscos principais:

- a linguagem pode ficar com dois modelos mentais competindo;
- exemplos simples podem comecar a parecer mais dificeis;
- bugs de memoria podem aparecer em codigo que antes era protegido por ARC;
- o backend C pode ficar mais complexo;
- o FFI pode ficar perigoso se ownership externo nao for bem documentado;
- `unsafe` pode se espalhar se virar atalho para performance;
- a equipe pode gastar tempo aqui antes de fechar prioridades mais importantes.

Por isso, a conversa chegou a uma recomendacao cautelosa:

- registrar a trilha;
- nao implementar agora como feature central;
- discutir novamente quando houver casos reais fortes;
- exigir RFC curta antes de qualquer codigo;
- comecar por `defer`, destrutores e arenas, nao por ponteiros crus.

## Decisao ainda aberta

Nada neste documento esta aprovado como surface publica.

Antes de implementar, a Zenith ainda precisa decidir:

- se gerenciamento manual opcional combina com a identidade da linguagem;
- qual custo tecnico e aceitavel;
- quais usuarios realmente precisam disso;
- qual parte deve ser segura por padrao;
- qual parte deve ficar em `std.unsafe`;
- se a feature entra antes ou depois de self-hosting, backend nativo e Borealis.

## Principio central

Memoria manual deve ser explicita, local e facil de reconhecer.

Contrato mental proposto:

```text
codigo comum = ARC automatico
codigo temporario = arena
codigo de performance = allocator explicito
codigo nativo/FFI = unsafe.ptr<T>
```

Se o leitor nao ve `std.mem` ou `std.unsafe`, ele nao deve precisar raciocinar
sobre liberacao manual.

## Nao-objetivos iniciais

Esta proposta nao tenta entregar:

- Rust completo;
- borrow checker completo;
- lifetimes explicitos na primeira versao;
- `malloc` e `free` como API comum;
- ponteiro cru fora de `std.unsafe`;
- substituicao do ARC existente;
- GC adicional;
- mudanca retrocompativel invisivel em `text`, `list`, `map` ou `dyn`.

## Beneficios esperados

### Performance previsivel

Alguns dominios precisam controlar quando memoria nasce e morre:

- engine e jogos;
- render;
- parser;
- compilador;
- serializacao binaria;
- buffers de rede;
- assets temporarios;
- ferramentas de build.

### Menos pressao sobre ARC

Em loops quentes, arenas e pools podem reduzir pares de retain/release e varias
alocacoes pequenas.

### FFI mais honesto

Interop com C, Raylib, bibliotecas nativas e APIs de sistema fica mais clara
quando a linguagem tem um vocabulario explicito para ponteiros, buffers e
ownership externo.

### Linguagem de sistemas acessivel

A Zenith pode manter o codigo comum simples e ainda permitir codigo de baixo
nivel quando o usuario pedir isso de forma explicita.

## Riscos

### Perda de legibilidade

Se ponteiro cru aparecer em codigo normal, a leitura fica mais dificil.

Mitigacao:

- manter `unsafe.ptr<T>` em namespace separado;
- priorizar `Arena` e `Allocator`;
- exigir docs com exemplos pequenos.

### Dois modelos de memoria ao mesmo tempo

ARC e memoria manual podem conflitar.

Mitigacao:

- bloquear managed values dentro de buffers manuais no MVP;
- documentar transferencia de ownership;
- criar testes negativos para misturas perigosas.

### Use-after-free, double-free e leaks

Memoria manual traz bugs que hoje o ARC evita.

Mitigacao:

- `DebugAllocator`;
- gates com sanitizers quando disponiveis;
- diagnosticos action-first;
- `defer` antes de expor uso manual amplo.

### FFI guardando ponteiro alem da chamada

O runtime atual protege chamadas FFI managed com retain/release. Ponteiros crus
exigem outro contrato.

Mitigacao:

- declarar no tipo se a funcao toma ownership, empresta ou escreve em buffer;
- manter o primeiro corte pequeno e voltado a buffers.

## Modelo recomendado

O caminho recomendado, se a proposta for aprovada, e hibrido:

1. ARC continua sendo o padrao.
2. `defer` e destrutores fecham recursos no fim do escopo.
3. `std.mem.Allocator` oferece controle explicito.
4. `ArenaAllocator` vem antes de `malloc/free`.
5. `DebugAllocator` detecta erros no desenvolvimento.
6. Containers ganham allocator opcional depois.
7. `unsafe.ptr<T>` fica restrito a FFI e baixo nivel.
8. `owned<T>` e `borrow<T>` ficam como possibilidade futura, nao como MVP.

## Exemplo de leitura desejada

Codigo comum:

```zt
const name = "Ana"
const items = list.of(1, 2, 3)
```

Codigo com arena:

```zt
use std.mem

const arena = mem.Arena.init()
defer arena.deinit()

const names = list.with_allocator<text>(arena.allocator())
names.push(mem.text_copy(arena.allocator(), "Ana"))
names.push(mem.text_copy(arena.allocator(), "Zenith"))
```

Codigo unsafe, apenas quando necessario:

```zt
use std.unsafe.memory as raw

const buffer = raw.alloc_bytes(1024)
defer raw.free(buffer)

raw.write_i32(buffer, 0, 42)
const value = raw.read_i32(buffer, 0)
```

## Fases propostas

### MM.M0 - RFC e contrato de linguagem

Status: futura, ainda a decidir.

- Definir se a Zenith quer memoria manual opcional.
- Definir nao-objetivos.
- Definir relacao com ARC.
- Definir relacao com FFI.
- Definir relacao com Borealis.
- Definir quais erros precisam de diagnostico amigavel.

### MM.M1 - `defer` e destrutores

Status: futura, ainda a decidir.

- Adicionar `defer`.
- Adicionar destrutores ou funcoes de fechamento padronizadas.
- Garantir ordem previsivel de execucao.
- Cobrir retorno antecipado e erro.
- Documentar uso com arquivos, sockets e arenas.

### MM.M2 - `std.mem.Allocator`

Status: futura, ainda a decidir.

- Criar interface minima de allocator.
- Criar `ArenaAllocator`.
- Criar `FixedBufferAllocator`.
- Criar `PageAllocator` ou equivalente de sistema.
- Manter surface pequena.

### MM.M3 - Debug allocator

Status: futura, ainda a decidir.

- Detectar leak em modo debug.
- Detectar double-free quando possivel.
- Detectar tamanho incorreto de free quando possivel.
- Gerar relatorio legivel.
- Integrar com testes.

### MM.M4 - Containers com allocator opcional

Status: futura, ainda a decidir.

- Adicionar construtores `with_allocator` para `text`, `bytes`, `list` e `map`.
- Preservar construtores atuais com ARC padrao.
- Validar isolamento entre valores ARC e valores alocados por arena.
- Medir impacto em performance.

### MM.M5 - Ponteiros crus restritos a `std.unsafe`

Status: futura, ainda a decidir.

- Definir `unsafe.ptr<T>` ou nome equivalente.
- Expor leitura/escrita de bytes e tipos primitivos.
- Proibir uso implicito como valor managed.
- Criar matriz FFI de permitido/bloqueado.
- Criar diagnosticos claros para uso fora de `unsafe`.

### MM.M6 - Ownership leve, se ainda fizer sentido

Status: futura, ainda a decidir.

- Explorar `owned<T>`.
- Explorar `borrow<T>`.
- Explorar `move` local.
- Explorar checker local por funcao.
- Adiar lifetimes completos para RFC separada.

## Criterios para aprovar a trilha

A proposta so deve virar implementacao se:

- houver pelo menos dois casos reais fortes;
- a surface couber em exemplos curtos;
- usuarios comuns puderem ignorar a feature;
- o runtime ARC continuar correto;
- o FFI ficar mais claro, nao mais perigoso;
- houver plano de diagnosticos e testes negativos;
- o custo nao competir com uma entrega mais importante da linguagem.

## Criterios para rejeitar ou adiar

A proposta deve ser rejeitada ou adiada se:

- exigir borrow checker completo cedo demais;
- tornar `text` e `list` mais dificeis de explicar;
- quebrar compatibilidade sem ganho claro;
- espalhar `unsafe` por codigo comum;
- nao houver benchmark ou caso real que justifique;
- o custo atrasar estabilizacao, docs, self-hosting ou Borealis.

## Decisao recomendada hoje

Nao implementar agora como feature central.

Manter como possibilidade futura.

Se for aprovada depois, comecar por:

1. `defer`;
2. destrutores;
3. `std.mem.Allocator`;
4. `ArenaAllocator`;
5. `DebugAllocator`;
6. `unsafe.ptr<T>` apenas para FFI.
