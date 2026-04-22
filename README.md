<p align="center">
  <img src="branding/logo-with-text.svg" width="420" alt="Zenith Language" />
</p>

# Zenith Language

Zenith is a reading-first language with explicit semantics and a native compilation pipeline.

## Current state

- Status: alpha published (v2 compiler track)
- Current version: `0.3.0-alpha.1` (2026-04-21)
- Alpha gate milestone: `R2.M12` closed

## Project origin (AI-assisted)

Zenith positions itself as one of the early language initiatives intentionally built with strong AI assistance, and as a practical case study in AI-assisted language engineering.

- The project is intentionally developed with heavy AI assistance.
- Primary goals include personal study, technical upskilling, and validation of an AI-assisted workflow.
- Language decisions, scope cuts, and release gates remain human-owned and explicitly documented.

What is intentionally deferred to the next cycle:

- Compass LSP productization
- public API expansion for `stdlib/platform/` (remains internal)

## Philosophy

Zenith follows four practical rules:

1. Readability before clever syntax.
2. Explicit behavior over implicit magic.
3. Stable formatter and diagnostics as language contracts.
4. Cognitive accessibility as a first-class requirement.

Core references:

- `language/spec/surface-syntax.md`
- `language/spec/cognitive-accessibility.md`
- `language/spec/formatter-model.md`
- `language/spec/diagnostics-model.md`
- `language/spec/implementation-status.md`

## What is implemented

- Compiler executable: `zt.exe`
- End-to-end pipeline: lexer -> parser -> AST -> semantic -> HIR -> ZIR -> C emitter -> native binary
- Project manifest: `zenith.ztproj`
- Runtime: `runtime/c/`
- Standard library: `stdlib/std/`
- Tooling: check, build, run, test, format, docs, summaries, perf gates

## Quick start

Requirements:

- Python 3
- GCC or Clang in `PATH`
- PowerShell, bash, or equivalent shell

Build:

```bash
python build.py
# or
bash build.sh
# or (Windows)
build.bat
```

Help:

```bash
./zt.exe
```

## First app

Project layout:

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

## Example projects (GitHub demo)

Curated runnable examples live in `examples/`:

- `examples/hello-world`
- `examples/structs-and-match`
- `examples/optional-and-result`
- `examples/multifile-imports`
- `examples/std-json`
- `examples/extern-c-puts`

Guide:

- `examples/README.md`

## CLI

- `zt check [project|zenith.ztproj]`
- `zt build [project|zenith.ztproj] [-o <output>]`
- `zt run [project|zenith.ztproj] [-o <output>]`
- `zt create [path|.] [--app|--lib] [--force]`
- `zt test [project|zenith.ztproj]`
- `zt fmt [project|zenith.ztproj] [--check]`
- `zt doc check [project|zenith.ztproj]`
- `zt doc show [symbol]`
- `zt summary [project|zenith.ztproj]`
- `zt resume [project|zenith.ztproj]`
- `zt perf [quick|nightly|scenario]`

## Quality and performance gates

Main validation:

```bash
python run_all_tests.py
```

`run_all_tests.py` now includes official tooling gates:

```bash
./zt.exe fmt tests/behavior/tooling_gate_smoke --check
./zt.exe doc check tests/behavior/tooling_gate_smoke
```

Performance:

- `tests/perf/gate_pr.ps1`
- `tests/perf/gate_nightly.ps1`
- `tests/perf/run_perf.py`

## Distribution model

Current distribution for pre-release `0.3.0-alpha.1` includes a published package:

- GitHub Release assets (outside source repository):
  - `zenith-0.3.0-alpha.1-windows-amd64.zip`
  - `zenith-0.3.0-alpha.1-windows-amd64.checksums.txt`

Linux packaging support in-repo:

- `python3 tools/build_linux_packages.py --version <x.y.z>`
- Generates `.deb`, `.rpm`, and `.pkg.tar.zst` artifacts under `dist/linux/`
- Installs global `ZENITH_HOME` via `/etc/profile.d/zenith.sh`

Alpha package includes:

- `zt.exe`
- runtime and stdlib required for execution
- quick-start and compatibility notes
- complete licensing package

Compatibility policy:

- before `1.0.0`, breaking changes are allowed between pre-releases
- deferred items are tracked in `docs/planning/roadmap-v2.md` and `docs/planning/checklist-v2.md`
- per-milestone compatibility log: `docs/reports/compatibility/`

## Licensing package

Zenith source code is dual-licensed:

- Apache-2.0
- MIT

You can choose either license (`Apache-2.0 OR MIT`).

License files:

- `LICENSE`
- `LICENSE-APACHE`
- `LICENSE-MIT`

Governance and contribution:

- `CONTRIBUTING.md`
- `TRADEMARK_POLICY.md`
- `docs/licensing/README.md`

Notes:

- contribution terms follow the same dual-license model
- code license does not grant trademark rights automatically

## Repository map

- `compiler/` -> frontend, semantic, IR, backend, driver, tooling
- `runtime/c/` -> runtime primitives and memory model
- `stdlib/` -> public standard library and docs
- `tests/` -> behavior, semantic, runtime, targets, perf
- `language/spec/` -> canonical language and tooling specs
- `docs/` -> planning, reports, guides, licensing and standards

## Documentation hub

- `docs/planning/roadmap-v2.md`
- `docs/planning/checklist-v2.md`
- `docs/planning/cascade-v2.md`
- `docs/planning/cascade-v1.md`

## Wiki

- https://github.com/raillen/zenithlang/wiki
- Source pages: `docs/wiki/`
