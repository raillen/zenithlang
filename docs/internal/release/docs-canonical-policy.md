# Documentation Canonical Policy

> Audience: maintainer, docs writer
> Surface: release engineering
> Status: Phase 7 policy

English docs are canonical.

Other languages are best-effort translations. They should help readers, but they
must not define behavior that conflicts with the English docs or the language
spec.

## Source Order

When two docs disagree, use this order:

1. `language/spec/` for language and compiler behavior.
2. `docs/public/en/` for public learning and reference docs.
3. `docs/reference/` for generated or knowledge-base reference material.
4. Other `docs/public/<lang>/` translations.
5. Internal planning docs.

Historical decisions remain useful context. They do not override current
canonical specs.

## Translation Rule

Translated docs may lag behind English.

When updating translations:

- keep examples close to the English version;
- avoid adding new semantics only in a translation;
- link back to the English canonical page when a page is partial;
- prefer short sections and clear examples.

## Release Check

Before a release candidate:

- new public behavior must exist in English docs first;
- translated pages must not claim stronger guarantees than English pages;
- known translation gaps can ship if they are marked as best-effort.

