<p align="center">
  <img src="branding/logo-with-text.svg" width="420" alt="Zenith Language" />
</p>

# Zenith Language

Zenith e uma linguagem **reading-first** com capacidade de sistemas.

Ela e focada em:

- intencao explicita;
- comportamento previsivel;
- design composicional;
- diagnosticos legiveis;
- documentacao curta e direta;
- acessibilidade cognitiva para pessoas com TDAH e dislexia.

> Zenith nao tenta vencer por sintaxe curta.
> Ela tenta ser clara o suficiente para ser lida, revisada, ensinada e mantida.

## Estado atual

Zenith ainda e uma linguagem em evolucao.

- Status publico: alpha.
- Versao fonte atual: `0.3.0-alpha.3` (2026-04-24).
- Compilador atual: trilha `v2`, implementada em C.
- Executavel principal: `zt.exe`.
- Package manager local: `zpm.exe`.
- Backend atual: C emitido + compilador nativo.
- Runtime atual: `runtime/c/`.
- Stdlib atual: `stdlib/std/`.

O repositorio contem codigo real, testes, stdlib, runtime, docs, exemplos e
relatorios. Mesmo assim, Zenith deve ser tratada como projeto alpha:

- APIs ainda podem mudar;
- algumas features sao experimentais;
- nem todo roadmap e promessa de implementacao;
- documentacao publica, referencia, spec e planos internos ficam separados.

## O que Zenith e

Zenith e um estudo pratico sobre compiladores, runtime, ferramentas e design de
linguagens.

Ela nasceu para investigar ideias sobre:

- compiladores;
- sistemas;
- runtime;
- stdlib;
- FFI;
- ferramentas de projeto;
- LSP e editor;
- packages;
- IA aplicada a engenharia de linguagem.

Zenith tambem e um laboratorio de aprendizado profundo. O objetivo principal e
aprender com rigor, construir com clareza e registrar as decisoes de forma
honesta.

## O que Zenith nao e

Zenith nao e uma promessa de produto pronto para producao.

Ela tambem nao tenta substituir Rust, C, C#, Python, JavaScript, Go, Zig, Nim ou
outra linguagem consolidada.

Comparacoes com outras linguagens existem para orientar decisoes tecnicas, nao
para declarar superioridade.

## Manifesto curto

Zenith segue quatro regras praticas:

1. Clareza acima de esperteza sintatica.
2. Explicito antes de magico.
3. Leitura antes de digitacao curta.
4. Acessibilidade cognitiva como requisito, nao como enfeite.

Isso afeta a linguagem e o tooling:

- blocos tem forma visual previsivel;
- mutacao deve ser visivel;
- ausencia e falha sao conceitos separados;
- diagnosticos devem dizer o problema e a proxima acao;
- exemplos devem ser pequenos;
- docs devem reduzir carga mental;
- features futuras precisam provar que preservam legibilidade.

Referencias principais:

- `docs/public/history-and-manifesto.md`
- `language/decisions/033-language-philosophy-and-manifesto.md`
- `language/spec/surface-syntax.md`
- `language/spec/diagnostics-model.md`
- `language/spec/formatter-model.md`
- `language/spec/legibility-evaluation.md`

## Documentacao

A documentacao foi reorganizada por publico-alvo.

Use esta entrada para navegar:

- `docs/DOCS-STRUCTURE.md`: como a documentacao e organizada.
- `docs/public/README.md`: guias para usuarios e site.
- `docs/reference/README.md`: referencias curtas e consultaveis.
- `docs/internal/README.md`: planejamento, reports e manutencao.
- `docs/wiki/`: fonte das paginas da GitHub Wiki.

Leitura recomendada para comecar:

- `docs/public/get-started/installation.md`
- `docs/public/get-started/first-project.md`
- `docs/public/get-started/daily-workflow.md`
- `docs/public/learn/README.md`
- `docs/public/language/core-tour.md`
- `docs/public/cookbook/README.md`

Referencia rapida:

- `docs/reference/language/syntax.md`
- `docs/reference/language/types.md`
- `docs/reference/language/modules-and-visibility.md`
- `docs/reference/language/errors-and-results.md`
- `docs/reference/cli/zt.md`
- `docs/reference/cli/zpm.md`
- `docs/reference/stdlib/modules.md`

## Projeto com IA

Zenith e desenvolvida com forte assistencia de IA.

Isso faz parte do estudo.

A IA ajuda a:

- revisar documentacao;
- criar testes;
- comparar alternativas;
- encontrar inconsistencias;
- acelerar implementacao;
- organizar reports e roadmaps.

Mas decisoes de linguagem, cortes de escopo, gates e direcao do projeto
continuam sendo responsabilidade humana. Por isso o repositorio preserva
decisions, specs, checklists e reports.

## O que ja existe

Superficie atual em alto nivel:

- lexer, parser, AST, semantic, HIR, ZIR e emitter C;
- projeto via `zenith.ztproj`;
- CLI `zt` para check, build, run, test, fmt, docs, summary e perf;
- modo single-file: `zt run arquivo.zt` sem necessidade de projeto;
- runtime C com valores managed e ARC;
- stdlib com texto, arquivos, JSON, validacao, math, random, collections,
  tests, OS/process e outros modulos;
- `optional<T>` e `result<T,E>`;
- `trait`, `apply` e `dyn<Trait>`;
- `public var` de namespace com mutacao controlada;
- FFI 1.0 documentado para o corte atual;
- ZPM MVP local;
- LSP/VSCode beta local;
- docs publicas, referencias e specs internas.

## Modelo de runtime

O modelo padrao atual usa ARC para valores heap-managed.

Resumo:

- valores managed comuns usam contagem de referencia nao atomica;
- o caminho padrao e single-isolate;
- compartilhamento entre threads de valores managed comuns nao e default;
- fronteiras devem usar copia, transferencia ou contratos explicitos;
- ciclos de referencia podem vazar memoria no alpha;
- ponteiros crus e memoria manual ampla seguem como assunto futuro, nao como
  surface publica atual.

Referencias:

- `runtime/c/README.md`
- `language/spec/runtime-model.md`
- `docs/internal/planning/manual-memory-roadmap-v1.md`

## Quick start

Requisitos:

- Python 3;
- GCC ou Clang no `PATH`;
- PowerShell, bash ou shell equivalente.

Build:

```bash
python build.py
```

Windows:

```bat
build.bat
```

Ajuda:

```bash
./zt.exe
```

## Modo rapido (single-file)

Voce pode executar um arquivo `.zt` avulso sem criar um projeto:

```zt
namespace script

import std.io as io

func main() -> result<void, core.Error>
    io.write("Hello from Zenith\n")?
    return success()
end
```

```bash
./zt.exe run hello.zt
./zt.exe check hello.zt
./zt.exe build hello.zt
```

O compilador cria um manifesto sintetico automaticamente. O arquivo precisa
declarar `namespace` e conter uma funcao `main`.

## Primeiro app (projeto completo)

Estrutura:

```text
my_app/
  zenith.ztproj
  src/
    app/
      main.zt
```

`my_app/zenith.ztproj`:

```toml
[project]
name = "my-app"
kind = "app"
version = "0.1.0"

[source]
root = "src"

[app]
entry = "app.main"

[build]
target = "native"
output = "build"
profile = "debug"
```

`my_app/src/app/main.zt`:

```zt
namespace app.main

import std.io as io

func main() -> result<void, core.Error>
    io.write("Hello from Zenith\n")?
    return success()
end
```

Rodar:

```bash
./zt.exe check my_app/zenith.ztproj
./zt.exe build my_app/zenith.ztproj
./zt.exe run my_app/zenith.ztproj
```

## CLI

Comandos principais:

```bash
zt check [project|zenith.ztproj|file.zt]
zt build [project|zenith.ztproj|file.zt]
zt run [project|zenith.ztproj|file.zt]
zt test [project|zenith.ztproj]
zt fmt [project|zenith.ztproj] [--check]
zt doc check [project|zenith.ztproj]
zt doc show [symbol]
zt summary [project|zenith.ztproj]
zt perf [quick|nightly|scenario]
```

ZPM:

```bash
zpm init
zpm add <package>
zpm install
```

Guia:

- `docs/public/tools/zpm-guide.md`

## Exemplos

Projetos executaveis ficam em `examples/`.

Exemplos principais:

- `examples/hello-world`
- `examples/structs-and-match`
- `examples/optional-and-result`
- `examples/multifile-imports`
- `examples/std-json`
- `examples/extern-c-puts`

Leia tambem:

- `examples/README.md`
- `docs/public/cookbook/README.md`

## Validacao

Gate amplo:

```bash
python run_all_tests.py
```

Gates rapidos usados com frequencia:

```bash
python build.py
./zt.exe check zenith.ztproj --all --ci
python run_suite.py smoke --no-perf
```

Perf:

```bash
tests/perf/gate_pr.ps1
tests/perf/gate_nightly.ps1
```

## Roadmaps e decisoes

Roadmaps e checklists ficam em `docs/internal/planning/`.

Entradas importantes:

- `docs/internal/planning/README.md`
- `docs/internal/planning/roadmap-v4.md`
- `docs/internal/planning/checklist-v4.md`
- `docs/internal/planning/roadmap-v6.md`
- `docs/internal/planning/checklist-v6.md`
- `docs/internal/planning/manual-memory-roadmap-v1.md`
- `docs/internal/planning/manual-memory-checklist-v1.md`

Decisoes de linguagem ficam em `language/decisions/`.

Specs normativas ficam em `language/spec/`.

## Distribuicao

Release alpha atual:

- `0.3.0-alpha.3`

Artefatos locais de release ficam em:

- `docs/internal/reports/release/0.3.0-alpha.3-release-report.md`
- `docs/internal/reports/release/0.3.0-alpha.3-notes.md`

Changelog:

- `CHANGELOG.md`

Antes de `1.0.0`, breaking changes podem acontecer entre pre-releases.

## Licenca

Zenith usa licenca dupla:

- Apache-2.0
- MIT

Voce pode escolher qualquer uma das duas: `Apache-2.0 OR MIT`.

Arquivos:

- `LICENSE`
- `LICENSE-APACHE`
- `LICENSE-MIT`

Contribuicao, marca e licenciamento:

- `CONTRIBUTING.md`
- `TRADEMARK_POLICY.md`
- `docs/public/licensing/README.md`

## Mapa do repositorio

- `compiler/`: frontend, semantic, IR, backend, driver e tooling.
- `runtime/c/`: runtime C e modelo de memoria.
- `stdlib/`: stdlib publica e zdocs.
- `language/spec/`: especificacao normativa.
- `language/decisions/`: decisoes com contexto.
- `docs/public/`: guias para usuarios.
- `docs/reference/`: referencias consultaveis.
- `docs/internal/`: planos, reports, governance e arquitetura.
- `docs/wiki/`: fonte da GitHub Wiki.
- `examples/`: projetos demonstrativos.
- `tests/`: suites de comportamento, semantic, runtime, formatter, LSP e perf.
- `tools/`: ferramentas auxiliares.
- `packages/`: packages oficiais em desenvolvimento, incluindo Borealis.

## Wiki

- https://github.com/raillen/zenithlang/wiki
- Fonte local: `docs/wiki/`
