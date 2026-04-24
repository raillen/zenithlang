# Raylib native binaries for Borealis

This folder is the preferred place for Raylib runtime binaries shipped with the
Borealis module.

Keep native binaries out of `src/`. Source files stay portable; platform files
live here.

Recommended layout:

```text
packages/borealis/native/raylib/
  windows-x64/
    raylib.dll
  linux-x64/
    libraylib.so
  macos-arm64/
    libraylib.dylib
  macos-x64/
    libraylib.dylib
```

Official release layout is also accepted:

```text
packages/borealis/native/raylib/
  windows-x64/
    include/
    lib/
      raylib.dll
      libraylibdll.a
```

Fallback layout, when a platform-specific folder is not needed:

```text
packages/borealis/native/raylib/
  raylib.dll
  libraylib.so
  libraylib.dylib
```

Runtime search order:

1. `BOREALIS_RAYLIB_PATH`
2. `ZENITH_RAYLIB_PATH`
3. default system loader path, such as `PATH` on Windows
4. executable directory
5. current working directory
6. `packages/borealis/native/raylib/<platform>`
7. `packages/borealis/native/raylib/<platform>/lib`
8. `packages/borealis/native/raylib/<os>`
9. `packages/borealis/native/raylib/<os>/lib`
10. `packages/borealis/native/raylib`
11. `packages/borealis/native/raylib/lib`

If Raylib is not found, Borealis keeps using the safe stub backend.
