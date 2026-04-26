# Borealis Engine + Studio Roadmap v3

> Roadmap especifico de Borealis Engine e Borealis Studio.
> Status: ativo para a trilha de engine/editor.
> Atualizado: 2026-04-25.

## Objetivo

Adicionar trabalho em segundo plano na engine sem inchar a linguagem Zenith.

A decisao deste roadmap e simples:

- a linguagem nao precisa ganhar `task`, `async` ou `chan<T>` para a engine funcionar bem;
- a engine pode ter um scheduler interno;
- o Studio pode expor fluxos simples para jobs e assets;
- o usuario deve usar APIs de alto nivel, nao primitives de concorrencia cruas.

## Problema

Borealis precisa carregar assets, preparar cenas e executar tarefas demoradas sem travar o editor ou o jogo.

Exemplos:

- carregar textura, modelo, audio ou shader;
- importar asset pesado;
- preparar navmesh;
- compilar script;
- validar cena;
- gerar preview;
- atualizar cache do Asset Browser.

Isso tem relacao direta com a engine e o Studio, nao com a semantica publica minima da linguagem.

## Principio central

A concorrencia deve ficar atras de APIs de engine.

O usuario nao deve precisar escrever:

```zt
task load_assets()
```

O caminho preferido e:

```zt
assets.request_load("hero.png")
```

ou:

```zt
jobs.request("build_navmesh")
```

A engine decide se roda agora, no proximo frame, em fila interna ou em worker nativo.

## Decisao de escopo

### Entra neste roadmap

- scheduler interno da engine;
- fila de jobs controlada pela engine;
- asset loading assinc-like via API de Borealis;
- progresso e diagnostics de jobs no Studio;
- Asset Browser com estado claro: queued, loading, ready, failed;
- Play Mode usando o mesmo contrato de runtime;
- cancelamento seguro para jobs de editor quando possivel.

### Nao entra neste roadmap

- keyword `task` na linguagem;
- `async func` na linguagem;
- `await` na linguagem;
- `chan<T>` como tipo da linguagem;
- `Shared<T>` exposto para usuario comum;
- raw threads no script de jogo.

Esses itens pertencem a RFC futura da linguagem, se um dia forem necessarios.

## Modelo recomendado

```text
Borealis Studio
  UI / Asset Browser / Inspector
        |
        v
Engine Job API
  jobs.request(...)
  jobs.poll(...)
  jobs.cancel(...)
        |
        v
Borealis Scheduler interno
  fila principal
  fila de assets
  fila de importacao
  fila de preview
        |
        v
Runtime / backend nativo
```

## API alvo de jobs

A API deve ser pequena e previsivel.

Exemplo conceitual:

```zt
const job: jobs.JobHandle = jobs.request("load_asset", "hero.png")
const state: jobs.JobState = jobs.state(job)
```

Estados minimos:

- `queued`
- `running`
- `done`
- `failed`
- `cancelled`

Regras:

- job nao acessa `public var` de jogo diretamente;
- resultado volta como handle, evento ou status;
- a engine controla quando aplicar resultado na cena;
- APIs de gameplay continuam main-thread por padrao.

## API alvo de assets async-like

O nome pode ser `request_load`, `preload` ou `load_deferred`.
A decisao final deve vir na implementacao.

Exemplo conceitual:

```zt
const request: assets.AssetRequest = assets.request_load("hero.png")

if assets.request_ready(request)
    const hero: assets.AssetHandle = assets.request_result(request)?
end
```

Regra de UX:

- para iniciantes, `asset_load(...)` sincronico continua existindo;
- para jogos maiores, `request_load(...)` evita travamento;
- o Studio mostra progresso e erro sem exigir que o usuario entenda threads.

## Studio

O Studio deve usar esse modelo para:

- importar assets sem congelar a UI;
- mostrar progresso no Asset Browser;
- cancelar importacao longa;
- validar cena em background;
- preparar preview antes de Play;
- exibir diagnostics separados por `studio`, `asset`, `runtime` e `compiler`.

## Relacao com async/await da linguagem

A linguagem Zenith atual nao tem `async func`/`await` implementado no compilador principal.

Portanto, Borealis nao deve depender disso.

Se `async/await` entrar no futuro, ele pode ser integrado como acucar sintatico por cima de jobs/assets.
Mas nao deve bloquear a engine.

## Fases

### B3.S0 - Contrato de jobs da engine

Objetivo:

- definir tipos, estados e lifecycle de jobs;
- documentar que jobs sao engine-owned;
- manter scripts de usuario livres de threads cruas.

### B3.S1 - Scheduler interno minimo

Objetivo:

- criar fila interna de jobs;
- executar jobs em passos previsiveis;
- permitir polling por estado;
- manter aplicacao de resultado no frame principal.

### B3.S2 - Asset requests

Objetivo:

- adicionar requests de loading/preload;
- expor progresso basico;
- preservar `AssetHandle` estavel;
- integrar com cache atual de assets.

### B3.S3 - Studio integration

Objetivo:

- mostrar jobs em painel simples;
- Asset Browser exibir queued/loading/ready/failed;
- permitir cancelamento quando seguro;
- manter UI responsiva.

### B3.S4 - Preview/Play Mode

Objetivo:

- preparar cena/assets antes do Play;
- sincronizar erros de job com diagnostics do Studio;
- nao travar o editor durante preparacao.

## Gates

- `./zt.exe check packages/borealis/zenith.ztproj --all`
- `./zt.exe run tests/behavior/borealis_foundations_stub/zenith.ztproj`
- `./zt.exe run tests/behavior/borealis_scene_entities_stub/zenith.ztproj`
- build/test do Studio quando a UI for alterada.

## Decisao final

Para Borealis, a prioridade e engine scheduler + APIs simples.

Nao e prioridade adicionar concorrencia nova na linguagem.
