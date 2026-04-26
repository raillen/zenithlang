# Borealis Engine + Studio Checklist v3

> Checklist operacional para scheduler interno, jobs e assets async-like.
> Roadmap: `docs/internal/planning/borealis-engine-studio-roadmap-v3.md`.
> Status: aberto.
> Atualizado: 2026-04-25.

## Regra de fechamento

Marque um item apenas quando houver evidencia.

Evidencia pode ser:

- codigo implementado;
- teste executavel;
- fixture Borealis passando;
- Studio buildando;
- documento de contrato atualizado.

## B3.S0 - Contrato de jobs da engine

- [ ] Criar decision curta: jobs pertencem a engine, nao a linguagem.
- [ ] Definir `JobHandle`.
- [ ] Definir `JobState` com estados: queued, running, done, failed, cancelled.
- [ ] Definir regra: resultado de job aplica no frame principal.
- [ ] Definir regra: job nao acessa `public var` de gameplay diretamente.
- [ ] Definir diagnostics de job: origem, mensagem, severidade, acao sugerida.

Criterio de aceite:

- o contrato permite implementar scheduler sem `async func`, `await`, `task` ou `chan<T>`.

## B3.S1 - Scheduler interno minimo

- [ ] Criar modulo ou camada interna de scheduler no Borealis.
- [ ] Criar fila de jobs.
- [ ] Criar funcao para registrar job.
- [ ] Criar funcao para consultar estado.
- [ ] Criar funcao para cancelar job quando seguro.
- [ ] Garantir que callbacks/resultados sejam aplicados no frame principal.
- [ ] Adicionar teste de ordem previsivel para jobs simples.

Criterio de aceite:

- um job simples pode sair de queued para done sem bloquear o loop principal.

## B3.S2 - Asset requests

- [ ] Definir `AssetRequest`.
- [ ] Adicionar API alvo: `request_load`, `request_ready`, `request_result` ou nomes finais equivalentes.
- [ ] Integrar request com cache atual de assets.
- [ ] Preservar `AssetHandle` estavel.
- [ ] Representar erro de asset como `result<T, core.Error>`.
- [ ] Adicionar teste para request concluida.
- [ ] Adicionar teste para request com erro claro.

Criterio de aceite:

- o usuario consegue pedir carregamento sem depender de `async/await` da linguagem.

## B3.S3 - Studio integration

- [ ] Mostrar estado de asset: queued, loading, ready, failed.
- [ ] Mostrar painel simples de jobs ativos.
- [ ] Mostrar erro de asset sem travar UI.
- [ ] Permitir cancelamento de importacao quando seguro.
- [ ] Manter Asset Browser sincronizado com o manifesto do projeto.
- [ ] Validar que abrir projeto grande nao congela a tela.

Criterio de aceite:

- o Studio consegue importar/listar assets com feedback visual claro.

## B3.S4 - Preview e Play Mode

- [ ] Preparar assets pendentes antes de Play.
- [ ] Bloquear Play com mensagem clara quando asset essencial falhar.
- [ ] Enviar diagnostics de jobs para o painel correto.
- [ ] Garantir que Stop/Pause nao deixe jobs zumbis.
- [ ] Validar que preview usa o mesmo contrato de cena salvo.

Criterio de aceite:

- Play Mode nao depende de estado oculto do editor.

## Fora de escopo deste checklist

- [ ] `async func` na linguagem.
- [ ] `await` na linguagem.
- [ ] keyword `task`.
- [ ] `chan<T>`.
- [ ] raw threads expostas ao usuario.
- [ ] `Shared<T>` como API comum de gameplay.

Esses itens nao devem bloquear o trabalho da engine.

## Gates recomendados

- [ ] `./zt.exe check packages/borealis/zenith.ztproj --all`
- [ ] `./zt.exe run tests/behavior/borealis_foundations_stub/zenith.ztproj`
- [ ] `./zt.exe run tests/behavior/borealis_scene_entities_stub/zenith.ztproj`
- [ ] build/test do Studio se houver mudanca em UI.
