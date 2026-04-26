# Language Maturity + Host Async Checklist v1

> Checklist operacional do roadmap de maturidade e assincronicidade via runtime host.
> Roadmap: `docs/internal/planning/language-maturity-host-async-roadmap-v1.md`.
> Status: aberto.
> Atualizado: 2026-04-25.

## Regra de uso

Marque apenas itens com evidencia.

Evidencia pode ser:

- documento atualizado;
- spec ou decision publicada;
- codigo implementado;
- teste executado;
- fixture passando;
- relatorio de fechamento.

## LHA.M0 - Decisao e limites

- [x] Criar roadmap de maturidade + host async.
- [x] Criar checklist operacional.
- [ ] Criar decision normativa curta: sem `async/await`, `task`, `chan<T>` no core atual.
- [ ] Atualizar `language/spec/runtime-model.md` com ponteiro para a estrategia host async.
- [ ] Atualizar `language/spec/concurrency.md` com nota sobre host scheduler.
- [ ] Atualizar docs de features nao implementadas para evitar expectativa de `async/await`.
- [ ] Garantir que exemplos publicos nao usem `async func` como se fosse recurso atual.

Criterio de aceite:

- leitor entende que assincronicidade pratica vem primeiro por host runtime.

## LHA.M1 - Contrato minimo de Host Jobs

- [ ] Definir `JobHandle`.
- [ ] Definir `JobState`.
- [ ] Definir `JobError`.
- [ ] Definir estados: queued, running, done, failed, cancelled.
- [ ] Definir se resultado e consultado por polling, evento ou ambos.
- [ ] Definir regra de cancelamento seguro.
- [ ] Definir regra de aplicacao de resultado no ponto seguro do host.
- [ ] Definir diagnostics de jobs com origem e acao sugerida.

Criterio de aceite:

- Borealis, web e GUI conseguem compartilhar o mesmo conceito de job sem nova sintaxe.

## LHA.M2 - Runtime host adapter

- [ ] Escolher local da camada host adapter.
- [ ] Implementar registro de job.
- [ ] Implementar consulta de estado.
- [ ] Implementar finalizacao de job.
- [ ] Implementar falha de job com erro legivel.
- [ ] Implementar cancelamento minimo.
- [ ] Implementar fila de diagnostics.
- [ ] Adicionar teste de ordem previsivel.
- [ ] Adicionar teste de erro.
- [ ] Adicionar teste de cancelamento, se houver suporte real.

Criterio de aceite:

- um host consegue criar e acompanhar job sem bloquear o loop principal.

## LHA.M3 - Borealis primeiro consumidor

- [ ] Conectar roadmap Borealis/Studio v3 a este contrato.
- [ ] Criar modulo/camada de jobs no Borealis.
- [ ] Criar `AssetRequest`.
- [ ] Criar API de request de asset.
- [ ] Preservar `AssetHandle` estavel.
- [ ] Integrar com cache atual de assets.
- [ ] Adicionar fixture Borealis para request concluida.
- [ ] Adicionar fixture Borealis para request com erro.
- [ ] Mostrar estados no Studio quando a UI for implementada.

Criterio de aceite:

- Borealis prova assincronicidade pratica sem `async func` nem `task`.

## LHA.M4 - Web/GUI segundo consumidor

- [ ] Escrever proposta de handlers web sincronos com runtime concorrente por baixo.
- [ ] Definir background jobs para web.
- [ ] Definir como GUI posta eventos no loop principal.
- [ ] Definir timers sem `await`.
- [ ] Criar exemplo web pequeno.
- [ ] Criar exemplo GUI pequeno ou pseudo-exemplo normativo.

Criterio de aceite:

- a estrategia funciona fora da engine.

## LHA.M5 - Avaliacao de sintaxe futura

- [ ] Coletar exemplos reais onde handles/polling ficaram repetitivos.
- [ ] Medir impacto no LSP e parser se `async/await` entrar.
- [ ] Medir impacto no runtime ARC.
- [ ] Medir impacto em docs e exemplos.
- [ ] Decidir se `async/await` e necessario.
- [ ] Se necessario, abrir RFC propria.
- [ ] Se nao necessario, manter recurso fora do core.

Criterio de aceite:

- decisao de sintaxe futura e baseada em uso real, nao em pressao por feature.

## Guardrails permanentes

- [ ] `public var` nao vira estado global concorrente.
- [ ] valores gerenciados comuns nao cruzam threads sem copia ou wrapper explicito.
- [ ] LSP nao sugere `async`, `await`, `task` ou `chan<T>` como feature atual.
- [ ] docs separam: atual, host runtime, futuro.
- [ ] exemplos publicos priorizam APIs simples.

## Fora de escopo deste checklist

- implementar `async func`;
- implementar `await`;
- implementar keyword `task`;
- implementar `chan<T>`;
- expor raw threads;
- expor mutex/condvar como API primaria;
- expor raw pointers publicos.

Esses itens so voltam com RFC propria.

## Gates recomendados

- [ ] `python build.py`
- [ ] `./zt.exe check zenith.ztproj --all --ci`
- [ ] `python run_all_tests.py`
- [ ] gates Borealis quando o consumidor for implementado
- [ ] teste LSP se completion/keywords forem alterados
