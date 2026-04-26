# Self-Hosting Checklist v1

> Audience: maintainer
> Status: draft
> Surface: internal
> Source of truth: planning

## Operating Rule

Do not replace the official compiler in this checklist.

Each item should either prove maturity or expose a concrete gap.

## SH0: Planning Baseline

- [ ] Confirm current compiler remains authoritative.
- [ ] Mark all self-hosted tools as experimental.
- [ ] Add this roadmap to planning indices.
- [ ] Define where experimental Zenith tools will live.
- [ ] Define validation command for each tool.
- [ ] Create a standard report template for self-hosting slices.

## SH1: Small Zenith Tools

- [ ] Pick 3 low-risk internal tools to write in Zenith.
- [ ] Implement tool 1 using only stable language features.
- [ ] Implement tool 2 using only stable language features.
- [ ] Implement tool 3 using only stable language features.
- [ ] Add behavior or golden tests for each tool.
- [ ] Document any stdlib gap found during implementation.
- [ ] Verify no new language feature was added only for tool convenience.

Suggested first tools:

- [ ] Simple package manifest checker.
- [ ] Syntax fixture index generator.
- [ ] Small docs metadata checker.

## SH2: Lexer Prototype

- [ ] Define token struct.
- [ ] Define span struct with file, line and column.
- [ ] Support identifiers.
- [ ] Support keywords.
- [ ] Support integer literals.
- [ ] Support float literals if current parser subset needs them.
- [ ] Support text literals.
- [ ] Support operators and punctuation.
- [ ] Support line comments.
- [ ] Support block comments if needed by current syntax.
- [ ] Report invalid characters.
- [ ] Add golden tests for valid token streams.
- [ ] Add negative tests for invalid input.
- [ ] Record text/bytes stdlib gaps.

## SH3: Parser Prototype

- [ ] Define minimal AST structs/enums.
- [ ] Parse `namespace` as first declaration.
- [ ] Reject declarations before `namespace`.
- [ ] Parse imports and aliases.
- [ ] Parse public/private top-level declarations.
- [ ] Parse `const` and `var` declarations.
- [ ] Parse function signatures.
- [ ] Parse simple function bodies.
- [ ] Parse `if` blocks.
- [ ] Parse `while` blocks.
- [ ] Parse `match` blocks.
- [ ] Parse `case 1, 2, 3` multiple-value cases.
- [ ] Parse `case value x` for optional values.
- [ ] Parse `case success(x)` and `case error(e)` for results.
- [ ] Add golden tests for AST output.
- [ ] Add negative tests for malformed namespace/import layout.

## SH4: Diagnostics Prototype

- [ ] Define diagnostic severity.
- [ ] Define diagnostic code.
- [ ] Define diagnostic span.
- [ ] Define primary message.
- [ ] Define optional hint.
- [ ] Render diagnostic output for terminal.
- [ ] Keep output concise and readable.
- [ ] Add golden tests for rendered diagnostics.
- [ ] Compare output with current compiler diagnostic style.
- [ ] Document required improvements in official diagnostics.

## SH5: Toy Semantic Checker

- [ ] Build a symbol table for one namespace.
- [ ] Detect duplicate declarations.
- [ ] Detect unknown symbols.
- [ ] Validate namespace/import consistency.
- [ ] Check simple assignment type mismatch.
- [ ] Check simple return type mismatch.
- [ ] Check private/public access boundary.
- [ ] Check external read of `public var`.
- [ ] Reject external write to `public var`.
- [ ] Add positive fixtures.
- [ ] Add negative fixtures.
- [ ] Track false positives and false negatives.

## SH6: Stdlib Maturity Review

- [ ] Review `std.text` against lexer/parser needs.
- [ ] Review `std.bytes` against file/token needs.
- [ ] Review `std.collections` against AST and symbol table needs.
- [ ] Review `std.fs` against project scanning needs.
- [ ] Review `std.io` against CLI needs.
- [ ] Review `std.json` against report/golden metadata needs.
- [ ] Review `std.format` against diagnostics needs.
- [ ] Decide which gaps are stdlib gaps, not language gaps.
- [ ] Update stdlib roadmap if required.

## SH7: Tooling Integration

- [ ] Define how experimental self-hosted tools are invoked.
- [ ] Decide whether tools live under `tools/`, `examples/` or a dedicated package.
- [ ] Add smoke test command for each tool.
- [ ] Ensure `zt check` can validate tool projects.
- [ ] Ensure package imports work without fragile relative paths.
- [ ] Document local usage for maintainers.

## SH8: Feasibility Report

- [ ] Summarize what worked.
- [ ] Summarize what blocked implementation.
- [ ] List stdlib gaps.
- [ ] List language gaps.
- [ ] List diagnostics gaps.
- [ ] List tooling gaps.
- [ ] Decide whether to continue prototypes or start bootstrap compiler planning.
- [ ] Record decision in `language/decisions/` if it changes language direction.

## Readiness Check Before Real Bootstrap

Do not start a real self-hosted compiler until all items below are true:

- [ ] Syntax rules are stable enough to freeze for one cycle.
- [ ] Namespace/import behavior is stable.
- [ ] `public var` semantics are stable.
- [ ] `optional` and `result` are stable in docs and tests.
- [ ] `match` behavior is stable, including multiple case values.
- [ ] Core stdlib APIs used by tooling are stable.
- [ ] Diagnostics are good enough to debug compiler-like code.
- [ ] Package validation is reliable.
- [ ] LSP can support real development on medium-size Zenith files.
- [ ] The current compiler can still build/check all official fixtures.

## Evidence Commands

Use these as baseline commands when relevant:

```powershell
python tools/check_docs_paths.py
git diff --check
.\zt.exe check zenith.ztproj --all --ci
```

Add tool-specific commands as each self-hosted tool is created.

## Done Definition For v1

This checklist is done when:

- at least one small Zenith tool exists and is tested;
- lexer prototype exists or is explicitly deferred with reason;
- stdlib gaps are documented;
- bootstrap feasibility has a written decision;
- no official compiler replacement was attempted prematurely.
