# Zenith Manual Memory Checklist 1.0

> Checklist operacional derivado de `docs/internal/planning/manual-memory-roadmap-v1.md`.
> Status: proposta futura, ainda a decidir.
> Criado: 2026-04-26.

## Como usar

1. Nao marque item sem evidencia em codigo, docs ou testes.
2. Nao transforme este checklist em compromisso de implementacao.
3. Antes de qualquer codigo, feche uma RFC curta.
4. Preserve a regra: manual deve ser opcional e visivel.
5. Se a feature piorar a legibilidade do codigo comum, volte para design.

## Gates obrigatorios se a trilha for aprovada

- [ ] `python build.py` verde.
- [ ] `./zt.exe check zenith.ztproj --all --ci` verde.
- [ ] `python run_suite.py smoke --no-perf` verde.
- [ ] Behavior tests positivos para cada surface publica.
- [ ] Behavior tests negativos para cada erro novo.
- [ ] Docs publicas atualizadas apenas quando a surface for aprovada.
- [ ] Spec ou decision atualizada antes de mudar semantica.
- [ ] Relatorio em `docs/internal/reports/` ao fechar cada milestone.

## MM.M0 - RFC e contrato de linguagem

- [ ] Escrever RFC curta de memoria manual opcional.
- [ ] Explicar por que ARC continua sendo o padrao.
- [ ] Definir que codigo comum nao deve precisar de `std.mem`.
- [ ] Definir que ponteiro cru fica em `std.unsafe`.
- [ ] Definir nao-objetivos iniciais.
- [ ] Definir termos: owned, borrowed, allocator, arena, raw pointer.
- [ ] Decidir se `defer` e pre-requisito.
- [ ] Decidir se destructors fazem parte da mesma trilha.
- [ ] Decidir relacao com FFI 1.0.
- [ ] Decidir relacao com Borealis.
- [ ] Registrar riscos de use-after-free, double-free e leak.
- [ ] Registrar exemplos acessiveis para usuarios novos.

Criterio de aceite:

- [ ] Um leitor entende o que sera automatico, manual e unsafe sem ler a conversa original.

Evidencia esperada:

- `language/decisions/`
- `language/spec/`
- `docs/internal/planning/manual-memory-roadmap-v1.md`

## MM.M1 - `defer` e destrutores

- [ ] Definir sintaxe de `defer`.
- [ ] Definir ordem de execucao de varios `defer`.
- [ ] Definir comportamento com `return`.
- [ ] Definir comportamento com `error(...)` ou propagacao de erro.
- [ ] Definir se `defer` pode capturar valores managed.
- [ ] Implementar parser.
- [ ] Implementar HIR/ZIR.
- [ ] Implementar backend C.
- [ ] Criar teste positivo simples.
- [ ] Criar teste com retorno antecipado.
- [ ] Criar teste com erro.
- [ ] Criar teste de ordem de execucao.
- [ ] Documentar uso com arquivo ou handle.

Criterio de aceite:

- [ ] `defer` fecha recursos de forma previsivel e legivel.

Validacao esperada:

- [ ] `python build.py`
- [ ] `./zt.exe check zenith.ztproj --all --ci`
- [ ] `python run_suite.py smoke --no-perf`

## MM.M2 - `std.mem.Allocator`

- [ ] Definir interface minima de allocator.
- [ ] Definir erro de alocacao.
- [ ] Definir ownership do bloco retornado.
- [ ] Criar modulo `std.mem`.
- [ ] Criar docs zdoc para `std.mem`.
- [ ] Implementar `ArenaAllocator`.
- [ ] Implementar `FixedBufferAllocator`.
- [ ] Implementar allocator de sistema ou page allocator.
- [ ] Criar exemplos pequenos.
- [ ] Criar testes positivos.
- [ ] Criar testes de erro de capacidade.
- [ ] Garantir que usuarios comuns nao precisem importar `std.mem`.

Criterio de aceite:

- [ ] Um programa consegue alocar memoria temporaria com arena e liberar tudo no fim do escopo.

Evidencia esperada:

- `stdlib/std/mem.zt`
- `stdlib/zdoc/std/mem.zdoc`
- `runtime/c/zenith_rt.c`
- `runtime/c/zenith_rt.h`
- `tests/behavior/manual_memory_*`

## MM.M3 - Debug allocator

- [ ] Definir formato de relatorio de leak.
- [ ] Definir comportamento de double-free.
- [ ] Definir comportamento de free com tamanho incorreto.
- [ ] Definir modo debug versus release.
- [ ] Implementar tracking de alocacoes.
- [ ] Criar teste de leak detectado.
- [ ] Criar teste de double-free detectado.
- [ ] Criar teste de relatorio legivel.
- [ ] Garantir mensagens action-first.
- [ ] Documentar como ativar no CLI.

Criterio de aceite:

- [ ] Erros comuns de memoria manual geram mensagens curtas, claras e acionaveis.

Validacao esperada:

- [ ] Suite dedicada de runtime C.
- [ ] Behavior tests com diagnosticos esperados.

## MM.M4 - Containers com allocator opcional

- [ ] Definir quais containers entram no primeiro corte.
- [ ] Adicionar `with_allocator` para `bytes`.
- [ ] Adicionar `with_allocator` para `text`, se aprovado.
- [ ] Adicionar `with_allocator` para `list<T>`.
- [ ] Adicionar `with_allocator` para `map<K,V>`, se aprovado.
- [ ] Preservar construtores atuais.
- [ ] Impedir mistura perigosa entre arena e ARC.
- [ ] Criar testes de arena liberando varios valores.
- [ ] Criar testes de ARC padrao sem regressao.
- [ ] Medir impacto em benchmark simples.

Criterio de aceite:

- [ ] Codigo existente continua funcionando sem alterar imports ou construtores.

Validacao esperada:

- [ ] Gate raiz.
- [ ] Smoke suite.
- [ ] Benchmarks antes/depois.

## MM.M5 - `std.unsafe` e ponteiros crus

- [ ] Definir nome do tipo: `unsafe.ptr<T>` ou alternativa.
- [ ] Definir funcoes permitidas no MVP.
- [ ] Definir leitura/escrita de bytes.
- [ ] Definir leitura/escrita de primitivas.
- [ ] Bloquear managed values no primeiro corte.
- [ ] Bloquear uso fora de `std.unsafe`.
- [ ] Criar diagnostico para ponteiro cru em codigo seguro.
- [ ] Criar matriz FFI permitido/bloqueado.
- [ ] Criar teste positivo de buffer nativo.
- [ ] Criar teste negativo para free de valor ARC.
- [ ] Criar teste negativo para uso unsafe fora de contexto permitido.

Criterio de aceite:

- [ ] Ponteiro cru existe para FFI e baixo nivel, mas nao aparece como recurso comum.

Evidencia esperada:

- `language/spec/ffi.md`
- `stdlib/std/unsafe/`
- `tests/behavior/unsafe_*`
- `tests/fixtures/diagnostics/unsafe_*`

## MM.M6 - Ownership leve

- [ ] Avaliar se `owned<T>` ainda e necessario.
- [ ] Avaliar se `borrow<T>` ainda e necessario.
- [ ] Avaliar `move` local.
- [ ] Definir erros para uso depois de move.
- [ ] Implementar apenas checker local por funcao, se aprovado.
- [ ] Adiar lifetimes completos.
- [ ] Criar exemplos pequenos.
- [ ] Criar testes negativos de uso depois de move.
- [ ] Criar testes negativos de borrow mutavel concorrente, se aplicavel.

Criterio de aceite:

- [ ] O modelo melhora seguranca sem transformar a Zenith em uma linguagem Rust-like completa.

## Perguntas abertas

- [ ] A Zenith precisa mesmo disso antes de self-hosting?
- [ ] Borealis precisa de arena no nivel da linguagem ou basta runtime/stdlib?
- [ ] `defer` deve entrar mesmo se memoria manual for adiada?
- [ ] `std.mem` deve ser publico ou experimental por varios ciclos?
- [ ] O primeiro corte deve aceitar apenas `bytes` e buffers?
- [ ] Como documentar isso para usuarios com TDAH e dislexia sem sobrecarregar?

## Sinais para prosseguir

- [ ] Dois ou mais casos reais dependem de allocator manual.
- [ ] Um prototipo mostra ganho mensuravel.
- [ ] A surface cabe em uma pagina curta de docs.
- [ ] Os diagnosticos ficam claros.
- [ ] A feature nao exige reescrever o runtime ARC.

## Sinais para adiar

- [ ] A proposta exige lifetimes completos.
- [ ] Exemplos ficam longos demais.
- [ ] `unsafe` aparece em codigo comum.
- [ ] Containers existentes ficam mais dificeis de explicar.
- [ ] O ganho de performance nao foi medido.
- [ ] O custo compete com estabilizacao, docs, self-hosting ou Borealis.

