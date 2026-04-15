# Zenith Pack API Spec

## Goal

Zenith Packs should be:

- easy to read
- easy to scan
- predictable
- TDAH-friendly
- dislexia-friendly
- safe by default

The API should help creators understand what a pack does without forcing them to read a framework.

## Core Principles

### 1. Host-first

The contract belongs to the Zenith IDE host, not to the authoring language.

That means:

- Zenith is the preferred authoring language
- TypeScript is an officially supported bridge
- both targets speak the same host API

### 2. Declarative first

Simple things should stay simple.

The first version of the API should favor:

- themes
- snippets
- commands
- tasks
- keymaps
- inspector sections

before exposing deeper runtime hooks.

### 3. Calm surface area

The API must be small enough to memorize.

Every namespace should answer a clear question:

- `pack`: who am I
- `log`: what happened
- `commands`: what can be triggered
- `workspace`: what files can I read or write
- `editor`: what document is active
- `tasks`: what command pipeline can I run
- `terminal`: what terminal session can I control
- `inspector`: what contextual UI can I add
- `storage`: what local data can I persist
- `ui`: what small feedback can I show

### 4. Permission clarity

Permissions must be readable by humans, not only by engineers.

Each permission should always carry a reason:

- `workspace.read`
- `workspace.write`
- `terminal.exec`
- `network`
- `lsp.spawn`
- `debug.attach`
- `ui.inspector`
- `ui.panel`

## Pack Kinds

### `theme-pack`

Used for:

- editor themes
- UI token sets
- accessibility themes
- low-stimulus themes

No runtime code is required.

### `language-pack`

Used for:

- language identity
- snippets
- formatter bindings
- runtime profile contributions
- semantic providers
- LSP bridge registration

### `tool-pack`

Used for:

- commands
- tasks
- file actions
- workflow helpers

### `ui-pack`

Used for:

- inspector sections
- status widgets
- Hub modules
- panel integrations

### `bridge-pack`

Used for:

- advanced TypeScript integrations
- JSON-RPC bridges
- DAP bridges
- LSP bridges
- networked services
- external process coordination

## Manifest

Every pack starts with `zenith-pack.json`.

```json
{
  "schema": 1,
  "id": "zenith.rust",
  "name": "Rust Pack",
  "version": "0.1.0",
  "kind": "language-pack",
  "category": "Languages",
  "tags": ["verified", "community"],
  "author": "Zenith Community",
  "description": "Rust runtime, tasks, snippets and inspector support.",
  "repository": "https://github.com/zenith-ide/rust-pack",
  "engines": {
    "zenith": "^0.2.0"
  },
  "entry": {
    "source": "src/main.zt",
    "output": "dist/index.js",
    "runtime": "zenith"
  },
  "activation": [
    { "on": "startup" },
    { "on": "file-extension", "value": ".rs" },
    { "on": "command", "value": "rust.runTests" }
  ],
  "permissions": [
    { "id": "workspace.read", "reason": "Read Cargo.toml and Rust files." },
    { "id": "terminal.exec", "reason": "Run cargo commands." },
    { "id": "lsp.spawn", "reason": "Start rust-analyzer." }
  ]
}
```

## Manifest Fields

### Identity

- `schema`: manifest schema version
- `id`: stable pack id
- `name`: human-readable name
- `version`: semantic version
- `kind`: pack kind
- `category`: Hub category
- `tags`: optional discovery tags
- `author`: display author
- `description`: short summary for Hub cards
- `repository`: source repository URL

### Compatibility

- `engines.zenith`: supported Zenith IDE range

### Entry

- `entry.source`: original source file
- `entry.output`: compiled artifact
- `entry.runtime`: `zenith`, `typescript`, or `declarative`

### Activation

Allowed activation triggers for v1:

- `startup`
- `command`
- `file-extension`
- `workspace-contains`
- `language`

### Permissions

Each permission entry must include:

- `id`
- `reason`

Optional future fields:

- `optional`
- `scopedTo`

## Declarative Contributions

The `contributes` block should power most packs before runtime code is needed.

```json
{
  "contributes": {
    "languages": [
      {
        "id": "rust",
        "label": "Rust",
        "extensions": [".rs"],
        "fileNames": ["Cargo.toml"],
        "runtime": "rust"
      }
    ],
    "commands": [
      {
        "id": "rust.runTests",
        "title": "Run Rust Tests",
        "category": "Rust"
      }
    ],
    "tasks": [
      {
        "id": "rust.cargo.test",
        "label": "Cargo Test",
        "command": "cargo test",
        "when": {
          "runtime": ["rust"]
        }
      }
    ],
    "inspectorSections": [
      {
        "id": "rust.runtime",
        "title": "Rust Runtime",
        "when": {
          "runtime": ["rust"]
        },
        "fields": [
          { "type": "text", "key": "toolchain", "label": "Toolchain" },
          { "type": "path", "key": "manifest", "label": "Cargo.toml" }
        ]
      }
    ]
  }
}
```

## Recommended Runtime API

The runtime surface should stay compact.

### `ctx.pack`

- `ctx.pack.info()`

Returns:

- id
- name
- version
- install path
- granted permissions

### `ctx.log`

- `ctx.log.info(message)`
- `ctx.log.warn(message)`
- `ctx.log.error(message)`

### `ctx.commands`

- `ctx.commands.add(definition)`
- `ctx.commands.run(id, args?)`

Recommended shape:

```ts
ctx.commands.add({
  id: "rust.runTests",
  title: "Run Rust Tests",
  run: async () => {
    await ctx.tasks.run("cargo test");
  }
});
```

### `ctx.workspace`

- `ctx.workspace.root()`
- `ctx.workspace.exists(path)`
- `ctx.workspace.readText(path)`
- `ctx.workspace.writeText(path, content)`
- `ctx.workspace.readJson(path)`
- `ctx.workspace.list(path)`
- `ctx.workspace.watch(path, handler)`

### `ctx.editor`

- `ctx.editor.activeFile()`
- `ctx.editor.open(path, options?)`
- `ctx.editor.save(path?)`
- `ctx.editor.format(path?)`
- `ctx.editor.selection()`
- `ctx.editor.replaceSelection(text)`

### `ctx.tasks`

- `ctx.tasks.add(definition)`
- `ctx.tasks.run(idOrCommand, options?)`
- `ctx.tasks.stop(taskId)`

### `ctx.terminal`

- `ctx.terminal.create(options?)`
- `ctx.terminal.write(sessionId, data)`
- `ctx.terminal.kill(sessionId)`

### `ctx.inspector`

- `ctx.inspector.addSection(definition)`
- `ctx.inspector.update(sectionId, data)`

### `ctx.storage`

- `ctx.storage.get(key)`
- `ctx.storage.set(key, value)`
- `ctx.storage.delete(key)`

### `ctx.ui`

- `ctx.ui.toast(definition)`

Optional later additions:

- `ctx.ui.panel.add(definition)`
- `ctx.ui.dialog.open(definition)`

### `ctx.http`

Requires `network`.

- `ctx.http.get(url, options?)`
- `ctx.http.post(url, body, options?)`

### `ctx.lsp`

Requires `lsp.spawn`.

- `ctx.lsp.addBridge(definition)`

### `ctx.debug`

Requires `debug.attach`.

- `ctx.debug.addAdapter(definition)`

## Outcome Model

To reduce surprise, the API should prefer explicit results over exception-heavy control flow.

```ts
type Outcome<T> =
  | { ok: true; value: T }
  | { ok: false; error: { code: string; message: string } };
```

This improves readability and keeps failure states easy to scan.

## Zenith-first Authoring Model

Zenith should be the recommended way to write packs.

Example:

```zenith
pack "zenith.rust" {
  use permissions {
    workspace.read
    terminal.exec
    lsp.spawn
  }

  command "rust.runTests" title "Run Rust Tests" {
    run task "cargo test"
  }

  inspector "rust.runtime" {
    show when runtime is "rust"

    field text "toolchain" label "Toolchain"
    field path "manifest" label "Cargo.toml"
  }
}
```

Design rules for Zenith pack syntax:

- one concept per line
- low symbol density
- direct words over abbreviations
- minimal punctuation noise
- readable aloud

## TypeScript Bridge Model

TypeScript should remain available for advanced integrations.

Example:

```ts
export default definePack({
  activate(ctx) {
    ctx.commands.add({
      id: "rust.runTests",
      title: "Run Rust Tests",
      run: async () => {
        await ctx.tasks.run("cargo test");
      }
    });

    ctx.inspector.addSection({
      id: "rust.runtime",
      title: "Rust Runtime",
      when: { runtime: ["rust"] },
      fields: [
        { type: "text", key: "toolchain", label: "Toolchain" },
        { type: "path", key: "manifest", label: "Cargo.toml" }
      ]
    });
  }
});
```

## Theme Pack Shape

Theme packs should be the easiest format to author.

```json
{
  "schema": 1,
  "id": "zenith.soft-focus",
  "name": "Soft Focus",
  "version": "0.1.0",
  "kind": "theme-pack",
  "category": "Themes",
  "tags": ["light", "tdah-friendly", "community"],
  "author": "Zenith Community",
  "description": "Low-noise light theme with softer contrast.",
  "entry": {
    "source": "theme.json",
    "runtime": "declarative"
  }
}
```

## Hub Categories

Recommended catalog categories:

- Languages
- Themes
- Tooling
- Git and DevOps
- Debug
- AI and Assistants
- Productivity
- Accessibility
- Education
- UI Panels

Recommended theme tags:

- dark
- light
- high-contrast
- low-stimulus
- tdah-friendly
- minimal
- verified

## Phased Delivery

### v1

- manifest
- permissions
- theme packs
- declarative contributions
- Zenith Pack transpile to JS
- TypeScript bridge packs

### v2

- panels
- Hub module contributions
- richer LSP and DAP bridges
- sandbox hardening
- update and rollback flow

## Recommendation

The recommended product path is:

1. ship `theme-pack`
2. ship declarative pack contributions
3. ship Zenith-authored packs compiled to JS
4. support TypeScript bridge packs for advanced integrations

That gives Zenith a clear identity without blocking ecosystem growth.
