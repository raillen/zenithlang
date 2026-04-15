# Changelog

Este e o changelog canonico do Zenith dentro da estrutura atual de documentacao.

## [Unreleased]

### Changed

- documentacao principal alinhada ao estado verificado da implementacao ativa
- roadmap de estabilizacao promovido como referencia do estado real
- narrativa de self-hosting completo removida dos docs centrais onde ela contradizia o repositorio atual

## [0.3.0] - 2026-04-11

### Added

- marco editorial Ascension para a linha 0.3.x
- aceleracao da trilha self-hosted em Zenith
- expansao de docs e narrativa de soberania tecnica
- distribuicao multiplataforma para `@windows`, `@linux` e `@macos`
- modos standalone e bundle
- suporte nativo a FFI para C e C++

### Changed

- a documentacao da epoca passou a tratar Ascension como linha principal
- o bootstrap em Lua deixou de ser apresentado como unica narrativa do projeto

### Note

- o estado verificado atual do repositorio deve ser lido em `docs/roadmap_estabilizacao.md`

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
