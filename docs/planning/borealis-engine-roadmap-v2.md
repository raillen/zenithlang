# Borealis Engine Roadmap v2

## Objetivo

Transformar o Borealis de package + editor inicial em uma game engine completa.

Meta principal:

- tudo que existe no package Borealis deve aparecer no editor quando fizer sentido;
- toda mudanca visual deve refletir na viewport;
- o botao Play deve rodar o projeto com o runtime real;
- o Studio deve ser distribuivel sem depender do repositorio `zenith-lang-v2`.

Comparacao de alvo:

- Unity: cena, hierarchy, inspector, assets, play mode, prefabs e build;
- Godot: projeto leve, cenas editaveis, scripts, nodes/componentes e execucao rapida;
- Game Maker: fluxo simples para criar jogo pequeno sem exigir arquitetura pesada cedo.

## Principio central

O editor nao deve inventar um modelo paralelo.

O contrato deve fluir assim:

1. `packages/borealis` define runtime, modulos e tipos.
2. O SDK publica metadados de editor.
3. O Studio le esses metadados.
4. A viewport mostra o estado editado.
5. O preview executa o mesmo projeto.
6. O build final usa o mesmo runtime.

Se uma funcao do package nao consegue aparecer em editor, preview ou build, ela ainda nao esta pronta como recurso de engine.

## Artefatos canonicos

O ciclo da engine precisa destes arquivos como fonte de verdade:

| Artefato | Dono | Funcao |
| --- | --- | --- |
| `packages/borealis` | runtime | API publica e comportamento real |
| `runtime/sdk/borealis.editor.json` | SDK | componentes, campos, assets, actions e panels do editor |
| `*.ztproj` | projeto | dependencias, cenas, scripts e build settings |
| `scenes/*.scene.json` | projeto | entidades, componentes e configuracao da cena |
| `assets/` | projeto | modelos, texturas, audio, shaders e dados |
| `tools/borealis-studio` | editor | UI, arquivos locais, viewport, preview e build |

## Modelo de cena necessario

O modelo atual de entidade + componentes deve crescer para cobrir tambem a cena.

Formato alvo:

```json
{
  "version": 2,
  "name": "Main",
  "document_id": "scene:main",
  "environment": {
    "skybox": {},
    "ambient": {},
    "fog": {},
    "weather": {}
  },
  "render": {
    "quality": {},
    "postfx": {},
    "camera": {}
  },
  "audio": {
    "listener": {},
    "mix": {}
  },
  "entities": []
}
```

Regra:

- `camera`, `light`, `sound emitter`, `model`, `sprite`, `script`: entidade/componente.
- `skybox`, `fog`, `weather`, `ambient`, `postfx`, `quality`: configuracao de cena.
- `input`, `save`, `storage`, `database`, `services`: projeto/runtime, nao viewport direta.

## Manifesto de editor

O package Borealis deve publicar um manifesto para o Studio.

Arquivo alvo:

```text
runtime/sdk/borealis.editor.json
```

Conteudo minimo:

```json
{
  "components": {},
  "sceneSettings": {},
  "assetKinds": {},
  "actions": {},
  "templates": {}
}
```

Exemplos de entrada:

```json
{
  "components": {
    "point_light": {
      "module": "borealis.game.render3d",
      "label": "Point Light",
      "viewport": "light",
      "fields": [
        { "key": "color", "kind": "color" },
        { "key": "intensity", "kind": "number", "min": 0, "max": 20 },
        { "key": "range", "kind": "number", "min": 0, "max": 1000 }
      ]
    }
  },
  "sceneSettings": {
    "postfx": {
      "module": "borealis.game.postfx",
      "fields": [
        { "key": "fxaa", "kind": "boolean" },
        { "key": "bloom", "kind": "number", "min": 0, "max": 5 }
      ]
    }
  }
}
```

Esse manifesto alimenta:

- Inspector;
- menu Add Component;
- Scene Settings;
- Asset Browser;
- templates;
- preview/runtime hydration.

## Reflexo na viewport

Cada recurso visual precisa de duas camadas.

1. Viewport de edicao:
   - usa Three.js;
   - mostra cena sem iniciar o jogo;
   - mostra gizmos, cameras, luzes, skybox, modelos, sprites e audio icons;
   - responde rapido ao Inspector.
2. Preview em Play:
   - usa Borealis runtime real;
   - roda scripts, input, update, physics, audio e render real;
   - devolve status, diagnostics e frame/output para o Studio.

Regra de aceite:

- editar campo no Inspector deve alterar a viewport em menos de um ciclo de UI;
- clicar Play deve aplicar a cena serializada ao runtime;
- Pause e Stop devem manter estado claro entre editor e runtime.

## Arquitetura de processos

```text
Borealis Studio
  React UI
  Three.js editor viewport
  Tauri commands
        |
        | JSONL stdio ou transporte binario futuro
        v
Borealis Preview Runtime
  Zenith project loader
  Borealis package
  Raylib/native backend
  frame/audio/input loop
```

O editor nunca deve depender diretamente do Raylib.
O runtime nunca deve depender do React.

## Roadmap

## E0 - Contrato vivo do engine

Objetivo:

- parar de manter catalogos manuais no Studio como fonte final.

Entregas:

- `borealis.editor.json` inicial no SDK;
- loader no Studio com fallback para o catalogo atual;
- schema para `components`, `sceneSettings`, `assetKinds`, `actions`, `templates`;
- validacao simples do manifesto.

Aceite:

- `camera3d`, `model3d`, `cube3d`, `audio3d`, `postfx`, `world3d`, `atmosphere`, `weather` aparecem via manifesto;
- se o manifesto faltar, o Studio continua abrindo com fallback.

## E1 - Viewport editavel de engine

Objetivo:

- transformar viewport de mock visual em viewport de edicao real.

Entregas:

- renderer Three.js lendo `SceneDocument`;
- cameras visiveis como frustum;
- luzes visiveis e editaveis;
- skybox e background;
- modelos, cubos, sprites e billboards;
- icons para audio, scripts e triggers;
- grid, transform gizmo e selection real.

Aceite:

- adicionar componente visual muda a viewport;
- mover entidade atualiza cena serializada;
- salvar e reabrir preserva os campos.

## E2 - Scene Settings

Objetivo:

- cobrir o que nao pertence a uma entidade.

Entregas:

- painel Scene Settings;
- `environment.skybox`;
- `environment.ambient`;
- `environment.fog`;
- `environment.weather`;
- `render.postfx`;
- `render.quality`;
- `audio.mix`.

Aceite:

- skybox, fog, ambient e postfx aparecem na viewport de edicao;
- Play recebe as mesmas configuracoes.

## E3 - Play mode real

Objetivo:

- Play deixa de ser status e vira execucao real do jogo.

Entregas:

- preview runner abre projeto criado pelo Studio;
- runtime carrega cena v2;
- scripts `update` rodam;
- input vai para o preview;
- Pause congela update;
- Stop volta ao estado de edicao;
- Console mostra logs, diagnostics e erros de script.

Aceite:

- projeto novo roda no Play sem depender do repo raiz;
- alterar script e salvar muda comportamento no proximo Play;
- erro de script aparece no Console com arquivo e linha quando disponivel.

## E4 - Asset pipeline

Objetivo:

- assets deixam de ser arquivos listados e viram recursos de engine.

Entregas:

- import de modelo, textura, audio, shader e cubemap;
- `assetKinds` vindos do manifesto;
- thumbnails e metadata;
- reimport;
- validacao de path;
- referencias usadas por componentes e scene settings.

Aceite:

- arrastar asset para cena cria entidade ou preenche campo correto;
- asset quebrado aparece como problema, nao como falha silenciosa.

## E5 - Component authoring

Objetivo:

- criar fluxo completo de adicionar/remover/configurar comportamento.

Entregas:

- Add Component por categoria;
- forms gerados pelo manifesto;
- campos ricos: color, vector2, vector3, asset, enum, number, boolean, text;
- valores default;
- unknown fields preservados;
- component presets.

Aceite:

- `point_light`, `directional_light`, `camera3d`, `audio3d`, `postfx profile`, `weather zone` podem ser configurados sem editar JSON manualmente.

## E6 - Project system completo

Objetivo:

- projeto Borealis vira uma unidade real como em Unity/Godot.

Entregas:

- tela inicial com recentes;
- abrir projeto por dialog nativo;
- criar projeto por template;
- project settings;
- build settings;
- dependency lock;
- SDK local resolvido por versao;
- templates vindos do SDK.

Aceite:

- Studio empacotado cria, abre, roda e salva projeto fora de `zenith-lang-v2`.

## E7 - Debug e diagnostics

Objetivo:

- tornar erro visivel e corrigivel.

Entregas:

- Problems panel real;
- diagnostics de manifesto, cena, asset, script e runtime;
- highlight de entidade com erro;
- logs separados por `studio`, `compiler`, `runtime`, `asset`, `audio`;
- profiler basico de frame.

Aceite:

- nenhum erro importante fica apenas no terminal;
- Play bloqueado explica o motivo e aponta o arquivo.

## E8 - Prefabs e reusable gameplay

Objetivo:

- permitir criar jogos maiores sem duplicar entidade manualmente.

Entregas:

- prefab file;
- instantiate prefab;
- override local;
- update prefab;
- nested prefabs simples;
- component presets compartilhaveis.

Aceite:

- criar uma luz, inimigo, camera rig ou player controller reutilizavel sem copiar JSON.

## E9 - Build e distribuicao do jogo

Objetivo:

- sair do editor para jogo executavel.

Entregas:

- Build Game;
- empacotar runtime, assets e scripts;
- targets iniciais Windows;
- pasta `dist-game`;
- logs de build;
- erro de dependencia claro.

Aceite:

- um projeto criado no Studio gera executavel jogavel.

## E10 - Engine hardening

Objetivo:

- estabilizar a engine para uso real.

Entregas:

- testes de roundtrip de cena;
- testes de manifesto;
- testes de preview runner;
- smoke de projeto criado pelo Studio;
- performance budget de viewport;
- release checklist do Studio.

Aceite:

- mudancas em package, Studio ou preview nao quebram fluxo criar -> editar -> play -> build.

## Prioridade imediata

Implementar nesta ordem:

1. `borealis.editor.json` e loader hibrido no Studio.
2. Expandir `SceneDocument` para `environment`, `render` e `audio`.
3. Fazer viewport Three.js refletir camera, luz, skybox, postfx e audio icons.
4. Fazer preview runner carregar a cena v2.
5. Criar Scene Settings.
6. Criar Add Component por manifesto.
7. Criar pipeline de SDK/release.

## Nao negociar

- O editor deve editar o mesmo contrato que o runtime usa.
- O Play deve usar runtime real.
- O SDK deve ser distribuivel sem o repo da linguagem.
- O editor deve preservar campos desconhecidos.
- Funcao nova do Borealis precisa ter caminho claro para editor, preview e build.

## Gates

Antes de marcar uma fase como pronta:

1. `npm run build` em `tools/borealis-studio`;
2. `cargo fmt --check` em `tools/borealis-studio/src-tauri`;
3. `cargo check` em `tools/borealis-studio/src-tauri`;
4. `cargo test` em `tools/borealis-studio/src-tauri`;
5. `./zt.exe check packages/borealis/zenith.ztproj --all`;
6. smoke manual ou automatizado:
   - criar projeto;
   - abrir cena;
   - editar componente visual;
   - ver mudanca na viewport;
   - Play;
   - Stop;
   - salvar;
   - reabrir.

