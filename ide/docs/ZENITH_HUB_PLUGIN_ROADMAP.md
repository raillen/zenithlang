# Zenith Hub Plugin Roadmap

## Vision
Zenith Hub will evolve from an internal guide into the official entry point for discovery, installation, updates, and creation of community packs.

The guiding principle is:

- GitHub as distribution infrastructure
- Zenith Hub as the user experience
- Zenith Language as the preferred authoring path
- TypeScript as the supported bridge for advanced integrations

## Product Principles

### 1. Zenith-first, not Zenith-only
- Plugin development should feel native in Zenith.
- TypeScript should remain available for advanced bridges, external protocols, and ecosystem adoption.
- Theme packs should be the easiest on-ramp for the community.

### 2. TDAH-friendly UX
- The catalog must be quiet, filterable, and easy to scan.
- Each pack card should answer three things quickly:
  - what it adds
  - whether it is trusted
  - what permissions it needs

### 3. Curated before open marketplace
- v1 should be a curated Hub backed by GitHub repositories and releases.
- Search across arbitrary GitHub repos should not be the default UX.
- Community packs can exist early, but the official registry should mediate trust and compatibility.

### 4. Declarative before imperative
- Themes, snippets, tasks, keymaps, and inspector sections should be easy to declare.
- Full runtime APIs should be added after the declarative layer is solid.

## Pack Types

### Theme Pack
- Community themes, token sets, accessibility themes, low-stimulus themes
- No runtime code required
- Best entry point for creators

### Language Pack
- Language identity, snippets, formatter, runtime profile, LSP bridge registration

### Tool Pack
- Commands, tasks, file actions, workflow helpers

### UI Pack
- Inspector sections, panel integrations, status widgets, Hub modules

### Bridge Pack
- Advanced integrations that need TypeScript, external processes, JSON-RPC, DAP, LSP, network, or system bridges

## Hub Catalog Categories

Primary categories:

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

Suggested tags:

- official
- verified
- community
- tdah-friendly
- low-stimulus
- high-contrast
- beginner-friendly
- minimal
- dark
- light

## Distribution Model

### Registry
- Maintain an official GitHub registry repository, for example `zenith-hub-registry`
- Each entry should contain:
  - id
  - name
  - author
  - kind
  - category
  - tags
  - repo URL
  - latest release asset URL
  - compatibility range
  - permissions
  - verification status

### Pack Repository
- Each pack can live in its own GitHub repository
- Recommended structure:
  - `zenith-pack.json`
  - `src/`
  - `README.md`
  - `CHANGELOG.md`
  - release asset such as `.zpk`

### Installation Flow
1. Zenith Hub downloads the registry index
2. User filters by category or tag
3. Hub shows compatibility and permissions
4. IDE downloads the release asset
5. IDE validates manifest, version, and integrity metadata
6. IDE installs and activates the pack

## Roadmap

### Phase 0: Foundations
- Define `zenith-pack.json`
- Define pack kinds and categories
- Define permissions model
- Define compatibility strategy by Zenith IDE version
- Define local installation directory and pack loading lifecycle

### Phase 1: Theme Ecosystem
- Implement Theme Pack manifest
- Add community theme catalog inside Zenith Hub
- Add filters for dark, light, high-contrast, tdah-friendly, verified
- Add install, uninstall, preview, and rollback for themes
- Add “Create Theme Pack” template

### Phase 2: Local Packs
- Support local sideloaded packs from disk
- Implement Pack Manager in Hub:
  - installed
  - updates
  - disabled
  - developer mode
- Add manifest validation and permission preview
- Add basic lifecycle hooks for pack activation

### Phase 3: Zenith-first SDK
- Define the Zenith Pack authoring model
- Add compiler or transpiler path from Zenith to TypeScript or JavaScript
- Ship official templates:
  - theme pack
  - language pack
  - tool pack
- Add hot reload for local development

### Phase 4: TypeScript Bridge SDK
- Support TypeScript packs as an official advanced path
- Add bridge APIs for:
  - LSP
  - DAP
  - terminal tasks
  - network integrations
  - external tools
- Keep permissions explicit and narrow

### Phase 5: GitHub-backed Zenith Hub
- Connect Hub to curated registry repository
- Add install from GitHub release asset
- Add update detection
- Add verification badges
- Add compatibility warnings

### Phase 6: Full Marketplace Experience
- Search by category, tags, and verified status
- Featured packs
- Official packs
- Community packs
- Developer onboarding section inside Hub
- Pack detail pages with permissions, screenshots, changelog, and source repo

## API Priorities

### v1 Declarative API
- themes
- snippets
- commands
- tasks
- keymaps
- inspector sections by schema

### v1 Runtime API
- commands.register
- tasks.run
- workspace.read
- workspace.write
- editor.activeFile
- terminal.run
- inspector.registerSection

### v2 Runtime API
- languages.register
- lsp.registerBridge
- debug.registerAdapter
- ui.registerPanel
- hub.registerModule

## Security Model

Permissions should be small and human-readable:

- workspace.read
- workspace.write
- terminal.exec
- network
- lsp.spawn
- debug.attach
- ui.inspector
- ui.panel

Rules:

- Ask for permissions up front during install
- Differentiate verified and unverified packs clearly
- Prefer sandboxed execution
- Warn when a pack requests terminal, network, or filesystem write access

## UX Inside Zenith Hub

Recommended sections:

- Installed
- Discover
- Themes
- Languages
- Tools
- Updates
- Developer

Each pack card should show:

- name
- short purpose
- category
- tags
- compatibility
- permissions summary
- install or update state

## Developer Experience

Official toolchain should include:

- `create-zenith-pack`
- local dev mode
- hot reload
- manifest validator
- pack bundler
- release packager

Templates:

- `theme-pack`
- `zenith-tool-pack`
- `zenith-language-pack`
- `typescript-bridge-pack`

## Immediate Next Steps

1. Define `zenith-pack.json`
2. Define pack kinds and permissions
3. Implement Theme Pack format
4. Add Hub catalog categories and tags in the UI
5. Add local pack installation
6. Design the Zenith Pack API and TS Bridge API side by side
