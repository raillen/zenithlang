# Zenith 1.0 Announcement Draft

> Status: draft
> Audience: public users and contributors

Zenith 1.0 is the first stability-focused release line for the language.

The goal is not to be the biggest language surface. The goal is a readable,
accessible and predictable foundation for small programs, tooling experiments,
standard library growth and package work.

## What Is Stable

- readable core syntax;
- explicit functions, control flow and imports;
- `optional<T>` and `result<T, E>`;
- collections and practical stdlib helpers;
- action-first diagnostics;
- project manifests with `zenith.ztproj`;
- ZPM lockfile and SemVer dependency checks;
- examples and public English docs.

## What Is Experimental

- Borealis package APIs;
- advanced console terminal controls;
- benchmark comparisons;
- translated docs.

## What Is Deferred

- manual memory APIs;
- LLVM backend;
- broad async/concurrency runtime;
- full implicit type inference;
- generic argument inference.

## Validation

The 1.0 release gate requires:

- local build and project check;
- smoke and PR gate suites;
- clean checkout validation on Windows, Linux and macOS;
- no known open P0/P1 blocker.

## Install

Use the installation guide in `docs/public/en/get-started/`.

For source validation:

```text
python build.py
./zt.exe check zenith.ztproj --all --ci
python run_suite.py smoke --no-perf
```

