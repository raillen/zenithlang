# Zenith Cookbook

> Audience: user
> Status: draft
> Surface: public
> Source of truth: no

## Objective

Answer practical questions:

"How do I do this in Zenith?"

The cookbook exists to transform language concepts into direct solutions.

## Scope

Includes recipes for common problems:

- absence without`null`;
- error without exception;
- secure union;
- abstract method;
- virtual dispatch;
- input validation;
- conversion to text;
- secure lookup;
- intentional failures;
- readable tests.

Does not include:

- normative spec;
- roadmap;
- historical design debate;
- future features as if they were current.

## Dependencies

- Upstream:
  -`docs/internal/planning/documentation-roadmap-v1.md`
  - `docs/internal/standards/user-doc-template.md`- Downstream:
  -`docs/public/README.md`
  - `docs/reference/language/`- Related Code/Tests:
  -`tests/behavior/MATRIX.md`

## Main content

Standard recipe format:

1. Problem.
2. Short answer.
3. Recommended code.
4. Explanation.
5. Common mistake.
6. When not to use.

Recommended starter recipes:

| Recipe | Use when |
| --- | --- |
|`absence-without-null.md`| a value may not exist |
|`errors-without-exceptions.md`| an operation may fail and the caller must decide |
|`safe-union-with-enum.md`| a value can have different formats |
|`abstract-methods-with-trait.md`| several types need to fulfill the same contract |
|`virtual-dispatch-with-dyn.md`| values ​​of different types must be called by the same contract |
|`partial-class-with-apply.md`| you want to separate methods without fragmenting the data |
|`callable-delegate.md`| you want to pass a function as a value |
|`zenith-equivalents-from-other-languages.md`| you know the concept in another language and want the equivalent Zenith |
|`intentional-failure-builtins.md`| you need to fail clearly with`check`, `todo`or`unreachable` |

## Validation

Before publishing a recipe:

```powershell
python tools/check_docs_paths.py
git diff --check
```

Examples should compile or be marked as illustrative.

## Update history

- 2026-04-25: initial cookbook index.
- 2026-04-26: added Zenith equivalents recipe for concepts from other languages.
- 2026-04-26: Added recipe for intentional crashes with`check`, `todo`and`unreachable`.
