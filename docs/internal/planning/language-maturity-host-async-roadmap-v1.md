# Language Maturity + Host Async Roadmap v1

> Roadmap focado em maturidade da linguagem e assincronicidade via runtime host.
> Status: ativo para planejamento.
> Atualizado: 2026-04-25.

## Decisao central

Zenith nao deve adicionar `async/await`, `task`, `chan<T>` ou raw threads ao core agora.

Assincronicidade deve nascer primeiro em runtimes host:

- Borealis Engine;
- Borealis Studio;
- frameworks web;
- bibliotecas GUI;
- tools de build/importacao.

A linguagem so deve ganhar nova sintaxe quando houver:

- repeticao real nas APIs;
- modelo de runtime estabilizado;
- impacto claro de legibilidade;
- testes e docs suficientes para manter compatibilidade.

## Por que isso importa

Maturidade nao e ter mais recursos.

Maturidade e:

- pouca superficie publica instavel;
- comportamento previsivel;
- docs coerentes;
- tooling que nao quebra;
- runtime com limites claros;
- caminho de evolucao sem pressa.

Adicionar concorrencia cedo demais afetaria:

- parser;
- typechecker;
- runtime ARC;
- stdlib;
- LSP;
- FFI;
- docs;
- exemplos;
- compatibilidade futura.

## Politica para o core da linguagem

O core da linguagem deve manter:

- execucao `single-isolate` por padrao;
- ARC nao-atomico para valores comuns;
- `public var` como estado de namespace, nao global concorrente;
- fronteiras explicitas por copia (`std.concurrent.copy_*`);
- FFI seguro, sem raw pointers publicos por enquanto;
- nenhuma sintaxe nova de async sem RFC.

Ficam fora do core atual:

- `async func`;
- `await`;
- keyword `task`;
- `chan<T>`;
- raw threads;
- mutex/condvar como caminho principal;
- `Shared<T>` como API comum de usuario.

## Modelo recomendado

```text
Codigo Zenith simples
  handlers, callbacks, update, commands
        |
        v
API de runtime host
  jobs.request/state/result/cancel
  assets.request_load/request_ready/request_result
  web.background_job(...)
  gui.post_event(...)
        |
        v
Scheduler do host
  fila de jobs
  event loop
  worker pool
  asset loader
  web dispatcher
        |
        v
Runtime Zenith atual
  ARC
  copy boundary
  managed values
  FFI shield
```

O runtime host toma decisoes de execucao.

O core Zenith mantem as regras de seguranca.

## Aplicacao por produto

### Borealis Engine

Objetivo:

- carregar assets sem travar o jogo;
- preparar cena/navmesh/cache em background;
- aplicar resultado no frame principal.

Direcao:

- scheduler interno da engine;
- `jobs.request(...)`;
- `assets.request_load(...)`;
- sem `async func` no script.

### Borealis Studio

Objetivo:

- UI responsiva;
- importacao de assets em background;
- preview/play mode sem travar;
- diagnostics claros por origem.

Direcao:

- job panel;
- Asset Browser com estados;
- cancelamento seguro;
- bridge com runtime/preview.

### Framework Web

Objetivo:

- handlers simples;
- servidor concorrente por baixo;
- background jobs para tarefas longas;
- I/O externo sem expor threads ao usuario.

Direcao inicial:

```zt
func create_user(req: web.Request) -> web.Response
    const name: text = web.form(req, "name")?
    users.create(name)?
    jobs.request("send_welcome_email", name)
    return web.redirect("/users")
end
```

### GUI

Objetivo:

- event loop;
- callbacks;
- timers;
- operacoes pendentes por handle.

Direcao:

```zt
func draw() -> void
    if ui.button("Importar")
        import_job = jobs.request("import_asset", selected_file)
    end

    if jobs.running(import_job)
        ui.label("Importando...")
    end
end
```

## Fases

### LHA.M0 - Decisao e limites

Objetivo:

- congelar a decisao de nao adicionar async syntax agora;
- registrar limites do core;
- alinhar Borealis, web e GUI em uma direcao comum.

Entrega:

- roadmap;
- checklist;
- decision curta se o tema virar contrato normativo.

### LHA.M1 - Contrato minimo de Host Jobs

Objetivo:

- definir o contrato compartilhado entre hosts.

Tipos conceituais:

- `JobHandle`;
- `JobState`;
- `JobError`;
- `JobResult<T>` quando houver suporte generico suficiente.

Estados minimos:

- `queued`;
- `running`;
- `done`;
- `failed`;
- `cancelled`.

### LHA.M2 - Runtime host adapter

Objetivo:

- criar uma camada host reutilizavel.

Responsabilidades:

- registrar job;
- consultar estado;
- concluir job;
- cancelar job;
- publicar diagnostics;
- aplicar resultado no ponto seguro do host.

### LHA.M3 - Borealis primeiro consumidor

Objetivo:

- validar o modelo em engine real antes de mexer na linguagem.

Entregas:

- `jobs` interno;
- asset request;
- teste de scheduler simples;
- Studio mostrando estados.

### LHA.M4 - Web/GUI segundo consumidor

Objetivo:

- provar que o modelo nao e especifico de game engine.

Entregas:

- proposta de `web` handlers sincronos com runtime concorrente;
- background jobs;
- GUI event loop com handles;
- exemplos pequenos.

### LHA.M5 - Avaliacao de sintaxe futura

Objetivo:

- decidir se `async/await` ainda e necessario.

Regra:

- so abrir RFC de sintaxe se jobs/handles ficarem repetitivos demais;
- a sintaxe deve ser acucar por cima do modelo ja validado;
- nada deve quebrar o codigo existente.

## Gates

Gates de maturidade:

- nenhum recurso novo entra sem spec ou decision;
- nenhum recurso de concorrencia muda `public var` implicitamente;
- nenhum host compartilha valor gerenciado comum entre threads sem copia ou wrapper explicito;
- docs mostram o que existe e o que e futuro;
- LSP nao sugere sintaxe que o compilador nao aceita.

Gates tecnicos:

- `python build.py`;
- `./zt.exe check zenith.ztproj --all --ci`;
- `python run_all_tests.py`;
- gates Borealis quando o primeiro consumidor for implementado.

## Resultado esperado

Zenith fica menor e mais madura.

Produtos host ganham assincronicidade pratica.

O core da linguagem ganha tempo para observar APIs reais antes de assumir nova sintaxe publica.
