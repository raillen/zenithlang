# Decision 008 - Backend Desktop Linker Hook v1

- Status: accepted
- Date: 2026-04-22
- Type: runtime / backend integration
- Scope: Borealis desktop backend hookup and safe fallback

## Summary

Define a stable runtime hook for desktop backends so Borealis can link Raylib/OpenGL incrementally without blocking onboarding.

## Decision

1. Runtime C exposes `zt_borealis_desktop_api` as the desktop adapter contract.
2. Runtime C exposes:
   - `zt_borealis_set_desktop_api(...)`
   - `zt_borealis_get_desktop_api(...)`
3. Runtime includes an initial Raylib desktop adapter loaded dynamically when the Raylib library is available.
4. If `backend_id=1` is requested and no desktop adapter is registered/available, runtime falls back to stub mode.
5. `borealis.game` and `borealis.engine` API surface remains unchanged for beginners.

## Rationale

This keeps the beginner path stable while unlocking a clean technical path for native desktop rendering.

## Consequences

Positive:

- linker integration is explicit and testable;
- onboarding stays functional even without native backend libs installed;
- migration to real Raylib adapter is incremental.

Tradeoff:

- requesting desktop backend without adapter currently runs on fallback stub path.

## Out Of Scope

1. Full Raylib/OpenGL adapter implementation details.
2. Packaging/distribution of native backend binaries.
