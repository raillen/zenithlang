# Changelog

Este e o changelog canonico do Zenith dentro da estrutura atual de documentacao.

## [Unreleased]

### Changed

- documentacao principal, portal e website alinhados ao estado oficial 100% self-hosted;
- front door, bootstrap, release e auditoria passaram a ser descritos pela linha vigente, sem reapresentar a transicao como estado atual;
- trilha Lua mantida como runtime, implementacao de referencia e recuperacao extraordinaria, sem disputar a face oficial do produto.

### Added

- consolidacao documental dos ciclos `selfhost-oficializacao`, `selfhost-100` e `selfhost-pos100`;
- politica explicita de artefatos e fechamento editorial do corte pos-100;
- sincronizacao do `zenith-website` com os docs canonicos atuais.

## [0.3.6] - 2026-04-15

### Added

- produto oficial marcado como 100% self-hosted;
- gate recorrente de release self-hosted e auditoria de legado sem eventos no caminho oficial;
- politica de artefatos para `.selfhost-artifacts/`, `.ztc-tmp/` e limpeza oficial;
- builtins oficiais endurecidos em modo estrito, com `zpm`, `zman` e `ztest` no fluxo suportado.

### Changed

- `ztc.lua` passou a operar em modo self-hosted estrito como front door oficial;
- a documentacao central foi reescrita para refletir o corte definitivo do produto;
- o website oficial passou a espelhar o estado atual da linguagem e dos compiladores.

## [0.3.0] - 2026-04-11

### Added

- marco editorial Ascension para a linha 0.3.x;
- aceleracao da trilha self-hosted em Zenith;
- expansao de docs e narrativa de soberania tecnica;
- distribuicao multiplataforma para `@windows`, `@linux` e `@macos`;
- modos standalone e bundle;
- suporte nativo a FFI para C e C++.

### Changed

- a documentacao da epoca passou a tratar Ascension como linha principal;
- o bootstrap em Lua deixou de ser apresentado como unica narrativa do projeto.

### Note

- para o fechamento dessa historia, leia `docs/roadmap/selfhost-oficializacao.md`, `docs/roadmap/selfhost-100.md` e `docs/roadmap/selfhost-pos100.md`.

## [0.2.8] - 2026-04-10

### Added

- `std.net`
- `std.http`
- `std.crypto`
- `std.reflect`
- `std.events`
- `std.collections`
- ampliacoes em `std.time`, `std.text` e `std.math`

### Changed

- `std.log` recebeu formato mais estruturado
- `std.os.process` avancou em captura e espera assincrona

## [0.2.5] - 2026-04-09

### Added

- `std.os`
- `std.fs`
- `std.fs.path`
- `std.json`

### Fixed

- shadowing no codegen
- parsing de blocos aninhados
- escapes em strings
- suporte ampliado a operadores e estruturas no backend Lua

## [0.2.0] - 2026-04-08

### Added

- lowering e IR
- constant folding
- desestruturacao em `match` e `var`
- genericos com restricoes
- traits com default impls
- contratos `where`
- `.ztproj`
- prelude automatica
- slicing e spread

### Changed

- `@campo` consolidado para acesso ao self
- `.` consolidado como base do chaining
- variaveis passaram a exigir tipo explicito
- `->` padronizado para retorno
