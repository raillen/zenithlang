# Especificacao: Self-Hosted Host ABI

Atualizado em: 2026-04-15

Este documento define a fronteira minima entre o compilador self-hosted canonico e o host Lua que ainda sustenta bootstrap, runtime e release.

## 1. Escopo

Este contrato cobre o recorte oficial do self-hosted:

- front door oficial: `ztc.lua` em modo self-hosted estrito;
- compilador canonico: `src/compiler/syntax.zt`;
- artefato promovido: `ztc_selfhost.lua`;
- manifesto de ABI: `tools/selfhost_abi.lua`;
- gate de release: `tools/selfhost_release.lua`.

Tudo que toca o host para sustentar esse recorte deve passar por nomes `zt.selfhost.*` explicitamente listados aqui.

## 2. Superficie ABI Congelada

### Metadados e introspecao

- `zt.selfhost.has_slot`
- `zt.selfhost.host_os`
- `zt.selfhost.compile_result_text`

### Texto

- `zt.selfhost.text_len`
- `zt.selfhost.text_slice`
- `zt.selfhost.text_replace`

### Valores e lowering

- `zt.selfhost.value_is_present`
- `zt.selfhost.empty_value`
- `zt.selfhost.fold_number_binary`

### IO e orquestracao

- `zt.selfhost.read_module_source`
- `zt.selfhost.run_cli`

## 3. Regra de Versionamento e Extensao

Uma nova entrada de ABI so pode nascer quando todos os pontos abaixo andam juntos:

- binding `extern func "zt.selfhost.*"` no compilador canonico;
- implementacao no runtime host;
- registro em `tools/selfhost_abi.lua`;
- documentacao neste arquivo;
- validacao verde em `lua tools/selfhost_release.lua`.

Regra de projeto:

- preferir solucao em linguagem/stdlib antes de abrir nova ABI;
- abrir ABI nova apenas para necessidade real de host;
- nao usar helper ad hoc fora do namespace `zt.selfhost.*` como atalho de implementacao;
- se uma entrada deixar de ser necessaria, ela deve sair do runtime, do manifesto e deste documento no mesmo corte.

## 4. Gate de Release

O comando oficial de promocao self-hosted e:

- `lua tools/selfhost_release.lua`

Esse gate precisa permanecer verde para um release se apresentar como validado pela trilha self-hosted. Ele verifica:

- consistencia entre manifesto ABI e `extern` do compilador;
- presenca das bindings de runtime para toda a ABI congelada;
- bootstrap deterministico com promocao para `ztc_selfhost.lua`;
- corpus minimo de `check`, `build`, `run` e smoke de CLI em modo estrito;
- ausencia de uso legado nas superficies oficiais auditadas.

## 5. Corpus Minimo do Gate

O manifesto atual promove este corpus:

- `lua ztc.lua --strict-selfhost --version`
- `lua ztc.lua --strict-selfhost check src/compiler/syntax.zt`
- `lua ztc.lua --strict-selfhost build demo.zt .selfhost-artifacts/release/selfhost-release-demo.lua`
- `lua ztc.lua --strict-selfhost run tests/stdlib/test_optional.zt`
- `lua ztc.lua --strict-selfhost zpm help`
- `lua ztc.lua --strict-selfhost zpm doctor`
- `lua ztc.lua --strict-selfhost zman list`
- `lua ztc.lua --strict-selfhost zman show std.core`
- `lua ztc.lua --strict-selfhost ztest --help`
- `lua ztc.lua --strict-selfhost ztest --grep parser_tests/test_parser.lua`

Leitura correta deste corpus:

- os builtins passam pelo front door oficial estrito;
- artefatos temporarios do gate ficam em `.selfhost-artifacts/`.

## 6. O que nao faz parte deste contrato

- shape interno de AST, binder ou emitter;
- detalhes internos de `ztc_selfhost.lua`;
- modulos `.zt` experimentais fora do caminho oficial;
- qualquer helper host que nao esteja nomeado em `zt.selfhost.*`.

## 7. Leitura complementar

- `current-core.md`
- `selfhost-artifacts.md`
- `decisions/002-selfhost-architecture-cutover.md`
- `decisions/004-selfhost-100-cutover.md`
- `../roadmap/selfhost-100.md`
- `../roadmap/selfhost-pos100.md`
