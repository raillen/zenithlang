<p align="center">
  <img src="branding/logo-with-text.svg" width="420" alt="Zenith Language" />
</p>

# Zenith Language

Zenith is a **reading-first** programming language project with systems-level
ideas.

Its main purpose is clear and limited:

- study;
- small projects;
- research;
- curiosity;
- experiments in language design, compilers, runtime, tooling, and accessible
  documentation.

Zenith is **not** a market language.

It is not trying to compete with Rust, C, C#, Python, JavaScript, Go, Zig, Nim,
or any other established language. Comparisons with other languages are used
only to learn, make better technical decisions, and explain trade-offs.

This repository is a learning and research workspace that also contains real
code, tests, tools, docs, examples, and release artifacts.

## Documentation Language

English is the default language for the public documentation of Zenith.

The writing style should stay:

- direct;
- short;
- concrete;
- friendly to readers with ADHD and dyslexia;
- focused on small steps instead of large blocks of text.

Portuguese notes may still exist in internal planning or historical material,
but user-facing documentation should default to English from this point forward.

## Current Status

Zenith is still evolving.

- Public status: alpha.
- Current source version: `0.4.1-alpha.1`.
- Latest packaged alpha release: `0.3.0-alpha.3` (2026-04-24).
- Current source tree: post-`0.3.0-alpha.3` local development, with R4/R6
  closure reports and the active v7 roadmap.
- Current compiler track: `v2`, implemented in C.
- Main executable: `zt.exe`.
- Local package manager: `zpm.exe`.
- Current backend: emitted C + native compiler.
- Current runtime: `runtime/c/`.
- Current standard library: `stdlib/std/`.

Treat Zenith as an alpha project:

- APIs may change;
- some features are experimental;
- not every roadmap item is a promise;
- public docs, reference docs, specs, and internal plans have different roles.

## What Zenith Is

Zenith is a practical study of programming language implementation.

It explores:

- compilers;
- syntax and semantics;
- diagnostics;
- runtime design;
- standard library design;
- FFI;
- package tooling;
- LSP and editor support;
- documentation for cognitive accessibility;
- AI-assisted language engineering.

The goal is to learn with rigor, build with clarity, and record decisions
honestly.

## What Zenith Is Not

Zenith is not a promise of a production-ready language.

It is not a startup product, a market replacement, or an attempt to prove that
one language should replace another.

You can use Zenith to learn, test ideas, write small programs, and explore how
a language toolchain fits together. You should not choose Zenith today because
you need a stable production ecosystem.

## Short Manifesto

Zenith follows four practical rules:

1. Clarity above clever syntax.
2. Explicit behavior before magic.
3. Reading before short typing.
4. Cognitive accessibility as a requirement, not decoration.

This affects both the language and the tooling:

- blocks should have a predictable visual shape;
- mutation should be visible;
- absence and failure should be separate concepts;
- diagnostics should explain the problem and the next useful action;
- examples should be small;
- docs should reduce mental load;
- future features must prove that they preserve readability.

Main references:

- `docs/public/history-and-manifesto.md`
- `language/decisions/033-language-philosophy-and-manifesto.md`
- `language/spec/surface-syntax.md`
- `language/spec/diagnostics-model.md`
- `language/spec/formatter-model.md`
- `language/spec/legibility-evaluation.md`

## Documentation Map

The documentation is organized by audience and purpose.

Start here:

- `docs/DOCS-STRUCTURE.md`: how the documentation is organized.
- `docs/public/README.md`: public user guides and website content.
- `docs/reference/README.md`: short reference material.
- `docs/internal/README.md`: planning, reports, and maintenance notes.
- `docs/wiki/`: source files for the GitHub Wiki.

Recommended first reading:

- `docs/public/get-started/installation.md`
- `docs/public/get-started/first-project.md`
- `docs/public/get-started/daily-workflow.md`
- `docs/public/learn/README.md`
- `docs/public/language/core-tour.md`
- `docs/public/cookbook/README.md`

Quick reference:

- `docs/reference/language/syntax.md`
- `docs/reference/language/types.md`
- `docs/reference/language/modules-and-visibility.md`
- `docs/reference/language/errors-and-results.md`
- `docs/reference/cli/zt.md`
- `docs/reference/cli/zpm.md`
- `docs/reference/stdlib/modules.md`

## AI-Assisted Project

Zenith is developed with strong AI assistance.

That is part of the study.

AI helps with:

- documentation review;
- test creation;
- alternative comparisons;
- inconsistency checks;
- implementation speed;
- reports and roadmap organization.

Language decisions, scope cuts, validation gates, and project direction remain
human responsibilities. That is why the repository keeps decisions, specs,
checklists, and reports.

## What Already Exists

High-level current surface:

- lexer, parser, AST, semantic analysis, HIR, ZIR, and C emitter;
- project files through `zenith.ztproj`;
- `zt` CLI for check, build, run, test, fmt, docs, summary, and perf;
- single-file mode: `zt run file.zt` without creating a project;
- C runtime with managed values and ARC;
- standard library modules for text, files, JSON, validation, math, random,
  collections, tests, OS/process, and more;
- `optional<T>` and `result<T,E>`;
- `trait`, `apply`, and `dyn<Trait>`;
- namespace-level `public var` with controlled mutation;
- documented FFI 1.0 for the current slice;
- local ZPM MVP;
- local LSP/VSCode beta;
- public docs, reference docs, and internal specs.

## Runtime Model

The current default model uses ARC for heap-managed values.

Summary:

- common managed values use non-atomic reference counting;
- the default path is single-isolate;
- sharing common managed values between threads is not the default;
- boundaries should use copy, transfer, or explicit contracts;
- reference cycles can leak memory in the alpha;
- broad raw-pointer and manual-memory surfaces are future topics, not the
  current public surface.

References:

- `runtime/c/README.md`
- `language/spec/runtime-model.md`
- `docs/internal/planning/roadmap-v7.md`

## Quick Start

Requirements:

- Python 3;
- GCC or Clang on `PATH`;
- PowerShell, bash, or an equivalent shell.

Build:

```bash
python build.py
```

Windows:

```bat
build.bat
```

Help:

```bash
./zt.exe
```

## Single-File Mode

You can run one `.zt` file without creating a project:

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

The compiler creates a synthetic manifest automatically. The file still needs a
`namespace` declaration and a `main` function.

## First Project

Structure:

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

Run:

```bash
./zt.exe check my_app/zenith.ztproj
./zt.exe build my_app/zenith.ztproj
./zt.exe run my_app/zenith.ztproj
```

## CLI

Main commands:

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

Guide:

- `docs/public/tools/zpm-guide.md`

## Examples

Runnable projects live in `examples/`.

Main examples:

- `examples/hello-world`
- `examples/structs-and-match`
- `examples/optional-and-result`
- `examples/multifile-imports`
- `examples/std-json`
- `examples/extern-c-puts`

Read also:

- `examples/README.md`
- `docs/public/cookbook/README.md`

## Validation

Broad gate:

```bash
python run_all_tests.py
```

Frequent quick gates:

```bash
python build.py
./zt.exe check zenith.ztproj --all --ci
python run_suite.py smoke --no-perf
```

Performance gates:

```bash
tests/perf/gate_pr.ps1
tests/perf/gate_nightly.ps1
```

## Roadmaps And Decisions

Roadmaps and checklists live in `docs/internal/planning/`.

Important entries:

- `docs/internal/planning/README.md`
- `docs/internal/planning/roadmap-v7.md`
- `docs/internal/planning/checklist-v7.md`
- `docs/internal/planning/borealis-roadmap-v1.md`
- `docs/internal/planning/borealis-checklist-v1.md`
- `docs/internal/planning/borealis-engine-studio-roadmap-v3.md`
- `docs/internal/planning/borealis-engine-studio-checklist-v3.md`

Language decisions live in `language/decisions/`.

Normative specs live in `language/spec/`.

## Distribution

Latest packaged alpha release:

- `0.3.0-alpha.3`

Local release artifacts for that packaged cut:

- `docs/internal/reports/release/0.3.0-alpha.3-release-report.md`
- `docs/internal/reports/release/0.3.0-alpha.3-notes.md`

Changelog:

- `CHANGELOG.md`

Before `1.0.0`, breaking changes may happen between pre-releases.

## License

Zenith uses dual licensing:

- Apache-2.0
- MIT

You may choose either license: `Apache-2.0 OR MIT`.

Files:

- `LICENSE`
- `LICENSE-APACHE`
- `LICENSE-MIT`

Contribution, trademark, and licensing:

- `CONTRIBUTING.md`
- `TRADEMARK_POLICY.md`
- `docs/public/licensing/README.md`

## Repository Map

- `compiler/`: frontend, semantic analysis, IR, backend, driver, and tooling.
- `runtime/c/`: C runtime and memory model.
- `stdlib/`: public standard library and zdocs.
- `language/spec/`: normative specification.
- `language/decisions/`: decisions with context.
- `docs/public/`: user-facing guides.
- `docs/reference/`: consultable reference material.
- `docs/internal/`: plans, reports, governance, and architecture.
- `docs/wiki/`: source files for the GitHub Wiki.
- `examples/`: demonstration projects.
- `tests/`: behavior, semantic, runtime, formatter, LSP, and perf suites.
- `tools/`: helper tools.
- `packages/`: official packages in development, including Borealis.

## Wiki

- https://github.com/raillen/zenithlang/wiki
- Local source: `docs/wiki/`
