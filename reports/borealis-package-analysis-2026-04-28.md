# Analise do package Borealis

Data: 2026-04-28

Escopo: `packages/borealis`, incluindo API publica, organizacao de modulos,
contratos de editor, cenas, exemplos, docs de arquitetura e validacao basica.

## Veredito curto

Borealis esta coerente como direcao de produto: ele ja separa bem a camada facil
`borealis.game`, a camada tecnica `borealis.engine`, o binding direto
`borealis.raylib` e o contrato de editor em `borealis.editor.json`.

O pacote tambem esta tecnicamente vivo. Os gates principais passam e a trilha
3D ja tem uma fatia real via Raylib, nao apenas documento.

O maior problema nao e falta de ambicao. E excesso de superficie publica.
Muitos helpers internos, mapas globais e funcoes de suporte aparecem como
`public`. Isso aumenta ruido, dificulta onboarding e bate contra a filosofia da
linguagem: clareza, leitura rapida, baixa carga cognitiva e APIs acessiveis.

## Evidencia executada

Comandos executados:

```text
./zt.exe check packages/borealis/zenith.ztproj --all
./zt.exe run tests/behavior/borealis_foundations_stub/zenith.ztproj
./zt.exe run tests/behavior/borealis_runtime_gameplay_stub/zenith.ztproj
./zt.exe run tests/behavior/borealis_render3d_basic/zenith.ztproj
./zt.exe run tests/behavior/borealis_scene_entities_stub/zenith.ztproj
```

Resultado:

- todos os comandos acima passaram;
- `borealis_render3d_basic` abriu Raylib real e validou janela/render 3D basico;
- depois das correcoes, o check principal passou limpo com `verification ok`;
- fixtures executaveis passaram; restam avisos nativos do C gerado, tratados
  como ruido de backend, nao como problema de API Zenith.

## Correcoes aplicadas nesta rodada

- estado global dos modulos Borealis deixou de ser `public var`;
- `packages/borealis/src/borealis` agora tem zero `public var`;
- `warning[name.similar]` foi ajustado no binder para nao avisar em pares
  convencionais de engine, como `position_x/position_y` e `color_r/color_g`;
- o mesmo aviso continua ativo para nomes realmente confusos, validado pela
  fixture `readability_warnings_pass`;
- aliases de import em minusculas, como `backend`, nao geram mais falso positivo
  contra tipos publicos em PascalCase, como `Backend`;
- fixtures longas de Borealis foram divididas em helpers menores.
- o checker agora permite acesso qualificado privado dentro do proprio prefixo
  do modulo, fechando o bloqueio que mantinha helpers internos como publicos;
- helpers internos de mapa/estado, `require_*`, `normalize_*`,
  `profile_value_key` e `emit_internal` deixaram de ser publicos;
- foi criada a politica `packages/borealis/api-public-policy-v1.md`;
- cenas de fixture foram migradas para v2 com `environment`, `render`, `audio`
  e `entities`;
- foi criada a validacao `python tools/validate_borealis_contracts.py`;
- `modular_scaffolds_v1.zt` saiu de `examples` e virou fixture de integracao;
- a maturidade 3D foi documentada por modulo em
  `packages/borealis/architecture-3d-summary.md`.

## Tamanho da superficie

Levantamento de `packages/borealis/src`:

- 48 arquivos `.zt`;
- cerca de 12.390 linhas;
- cerca de 1.172 funcoes publicas;
- 159 structs publicas;
- 17 enums publicos.

Esse tamanho e aceitavel para uma engine em crescimento, mas nao e aceitavel
como superficie publica sem curadoria. Hoje o usuario iniciante ve, ou pode
importar, muitos detalhes de implementacao.

## Coerencia com a filosofia da linguagem

Pontos fortes:

- a camada facil (`borealis.game`) tem um fluxo claro: start, running,
  frame_begin, draw, frame_end, close;
- exemplos pequenos como `minimal_loop.zt` ainda mostram bem o caminho feliz;
- erros sao textuais e especificos por modulo, como `assets: path cannot be empty`;
- o manifesto `zenith.ztproj` declara perfil de acessibilidade `beginner`;
- settings inclui preferencias de acessibilidade como alto contraste, reduzir
  movimento e fonte para dislexia;
- docs explicam limites conhecidos em pequenos blocos.

Pontos fracos:

- os principais nomes internos deixaram de vazar para a API: `*_or`,
  map-copy `*_set`, `set_*`, `*_internal`, `require_*`, `normalize_*`,
  `profile_value_key` e `emit_internal` agora sao privados;
- mapas globais tambem ja foram removidos da superficie publica;
- `name.similar` ja nao dispara em padroes comuns de engine (`x/y/z`, `r/g/b/a`,
  `position_x/position_y/position_z`);
- exemplos de integracao foram separados: `modular_scaffolds_v1.zt` agora vive
  em `packages/borealis/fixtures`;
- a API mistura estilos: algumas partes usam verbos curtos (`move`, `push`),
  outras usam prefixos longos (`entity_set_position`,
  `render3d_set_quality_profile`), e outras parecem binding nativo cru.

## Organizacao de codigo

O desenho modular e bom:

- `game` fica como entrada simples;
- `engine` guarda controle tecnico e ECS;
- `raylib` e uma ponte explicita, util para quem quer chegar perto do backend;
- `game/*` cobre gameplay, dados, UI, debug, editor metadata e 3D experimental;
- `decisions/modules` e `decisions/modules3d` ajudam a manter intencao por modulo.

O mapa de dependencias tambem esta coerente. O codigo confirma poucas
dependencias diretas: `entities` depende de `engine.ecs`, `scene` depende de
`entities`, e a maior parte dos modulos usa estado proprio.

Problema anterior fechado: a linguagem agora aceita helper privado chamado por
prefixo proprio do modulo. Com isso, o Borealis pode manter implementacao
interna qualificada sem abrir essa superficie para o usuario.

## Funcionalidade atual

Funcional de verdade:

- loop 2D basico, input por frame e desenho basico;
- scene/entities com id estavel, tags, hierarquia, snapshot e dirty flag;
- ECS hibrido simples;
- assets 2D com cache, ids estaveis, loaders tipados e metadata;
- save/storage/settings/events/debug com comportamento deterministico;
- Raylib desktop com fallback seguro;
- 3D basico com `render3d_begin/end`, grid, cube, model e billboard.

Funcional, mas ainda com cara de scaffold:

- database e services sao contratos stub;
- varios modulos 3D guardam estado e debug, mas nao entregam simulacao/render
  completo;
- physics3d, ai3d, animation3d, controllers3d e ui3d ainda parecem mais
  contrato futuro do que engine completa;
- `draw_mesh` em `render3d` retorna erro explicito de nao implementado.

Isso esta coerente com a documentacao, que chama parte da trilha 3D de
experimental. O risco foi reduzido pela tabela de maturidade 3D, que separa
`runtime-backed`, `state-backed`, `stub` e `design-only`.

## Contrato de editor

`packages/borealis/borealis.editor.json` e um bom passo arquitetural.

Ele ja descreve:

- componentes de entidade: camera, luz, model, cube, sprite, audio, script;
- configuracoes de cena: skybox, ambient, fog, weather, postfx;
- tipos de asset;
- acoes iniciais.

Isso combina com a regra principal do roadmap: o editor nao deve inventar um
modelo paralelo.

Lacunas:

- cenas de fixture agora estao em v2 e carregam o formato alvo com
  `environment`, `render` e `audio`;
- o manifesto esta mais rico que o runtime em alguns pontos, por exemplo luzes,
  fog e postfx ainda nao tem reflexo completo em viewport/play, mas agora isso
  esta classificado por maturidade;
- o schema do manifesto agora tem validacao automatica para evitar drift entre
  package, Studio e preview.

## Coerencia de sintaxe e nomes

Bom:

- nomes como `game.start`, `game.frame_begin`, `game.draw_rect`,
  `entities.entity_create` e `scene.scene_push` sao previsiveis;
- structs pequenas (`Vector2`, `Color`, `WindowConfig`) sao faceis de ler;
- `result` e `optional` sao usados de forma consistente.

Ruim:

- prefixos repetidos dentro do proprio namespace deixam alguns nomes pesados:
  `entities.entity_*`, `scene.scene_*`, `render3d.render3d_*`;
- nomes de binding nativo expostos no mesmo pacote (`zt_borealis_*`) sao
  necessarios internamente, mas nao deveriam guiar a experiencia publica;
- funcoes auxiliares publicas como `event_text_or`, `set_float`, `bool_or` e
  `asset_take_stamp` nao expressam dominio de game engine para o usuario final.

Recomendacao de naming:

- manter nomes explicitos nos modulos publicos;
- esconder helpers de mapa/estado;
- manter a camada facil com frases curtas;
- aceitar que binding Raylib tenha nomes tecnicos, mas isolar isso como camada
  avancada.

## Principais riscos

1. Superficie publica ainda grande demais.
   A superficie foi reduzida, mas a curadoria final de nomes publicos ainda deve
   continuar antes de estabilizar `0.1.0`.

2. Maturidade 3D desigual.
   O slice Raylib e real, mas varios modulos seguem `state-backed`, `stub` ou
   `design-only`.

3. Viewport/play ainda precisam aplicar todos os campos de cena v2.
   O contrato existe; a paridade completa de luz, fog, postfx e audio ainda
   precisa crescer por modulo.

4. Binding nativo ainda e camada avancada.
   `borealis.raylib` continua util, mas nao deve guiar onboarding.

5. Cenas v2 ainda nao tem loader runtime completo.
   A fixture e a validacao existem; a execucao direta de todo o documento ainda
   e trabalho de runtime/editor.

## Recomendacao priorizada

1. Criar uma politica de API publica para Borealis.
   Feito em `packages/borealis/api-public-policy-v1.md`.

2. Reduzir a superficie publica dos helpers.
   Feito para mapas globais, helpers de mapa/estado, validadores internos,
   normalizadores internos e construtores de chave.

3. Resolver ou classificar warnings de legibilidade.
   Eixos, canais de cor e alias de import ja foram classificados no binder.
   Para nomes realmente confusos, manter o warning e renomear.

4. Separar exemplos de aprendizado e fixtures de integracao.
   Feito: `modular_scaffolds_v1.zt` agora esta em `packages/borealis/fixtures`.

5. Evoluir cena v2 antes de expandir mais componentes.
   Feito no contrato de fixture e validacao; proximo passo e aplicar todos os
   campos no runtime/editor.

6. Marcar modulos 3D por maturidade.
   Feito em `packages/borealis/architecture-3d-summary.md`.

7. Manter `packages/borealis` como fonte de contrato.
   O Studio deve continuar consumidor do package/SDK, sem modelo paralelo.

## Conclusao

Borealis esta no caminho certo como game engine Zenith. A arquitetura esta
alinhada com Unity/Godot/Game Maker em termos de contrato: cena, entidades,
componentes, assets, editor metadata, preview e runtime.

Mas para ficar coerente com a filosofia da linguagem, o proximo trabalho nao
deve ser apenas adicionar mais modulos. Deve ser curar a API, reduzir ruido,
separar exemplos didaticos de fixtures e transformar o contrato de cena v2 em
fonte real para editor, viewport, play mode e build.
