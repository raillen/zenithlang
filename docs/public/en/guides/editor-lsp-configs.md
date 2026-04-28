# Editor LSP Configs

This guide publishes local snippets for using `zt-lsp` outside VSCode.

## Before Setup

Build the LSP from the repository root:

```powershell
python tools/build_lsp.py
```

Expected result:

```text
SUCCESS
```

On Windows, this creates `zt-lsp.exe`.

On Linux/macOS, this creates `zt-lsp`.

If the editor cannot find the binary, use an absolute path.

## Neovim

Published snippet:

```text
tools/editor-configs/neovim/init.lua
```

Quick use:

1. Add the snippet to your Neovim config.
2. Adjust the path if needed:

```lua
vim.g.zenith_lsp_path = "C:/path/to/zt-lsp.exe"
```

The snippet:

- registers `.zt` files as `zenith`;
- finds the root through `zenith.ztproj` or `.git`;
- starts `zt-lsp` for `.zt` buffers.

## Helix

Published snippet:

```text
tools/editor-configs/helix/languages.toml
```

Quick use:

1. Open or create Helix `languages.toml`.
2. Add the snippet block.
3. Replace `command = "zt-lsp"` with an absolute path if needed.

## Zed

Published snippet:

```text
tools/editor-configs/zed/settings.json
```

Quick use:

1. Open Zed `settings.json`.
2. Merge the snippet block with your current settings.
3. Replace `"path": "zt-lsp"` with an absolute path if needed.

## Quick Validation

Open a `.zt` file.

Then check:

- diagnostics appear when there is an error;
- hover and completions respond;
- the project root has `zenith.ztproj`.

## Current Limits

- The LSP still uses full-document sync.
- The binary must be built locally.
- The VSCode extension remains the most complete path in this cut.
