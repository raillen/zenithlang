# Changelog

Este e o changelog canonico do Zenith dentro da nova estrutura de documentação.

## [Unreleased]

### Changed

- Ajustes de documentação e polimento editorial após a conclusao do motor Ascension.

## [0.3.0] - 2026-04-11

### Added

- motor Ascension concluído como compilador auto-hospedado oficial
- pipeline nativo consolidado para build, run e testes
- otimizações no próprio motor, incluindo constant folding e inlining
- distribuição multiplataforma para `@windows`, `@linux` e `@macos`
- modos standalone e bundle
- suporte nativo a FFI para C e C++

### Changed

- Zenith passa a se compilar com Zenith
- o bootstrap em Lua deixa de ser a linha principal e passa a ser legado preservado
- o site e a documentação foram atualizados para refletir a auto-hospedagem como estado atual

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
- `std.os.process` avancou em captura e espera assíncrona

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
- variaveis passaram a exigir tipo explícito
- `->` padronizado para retorno

## [0.1.0] - 2026-03-02

### Added

- primeira versão funcional da linguagem
- variaveis, funções e structs básicas
- transpilation inicial para Lua