# Zenith Manifesto: Sovereignty and Cognitive Accessibility

This document defines the identity, technical philosophy, and strategic vision of the Zenith language. Zenith is not just a programming tool; it is a cognitive infrastructure designed to transform complexity into clarity.

## 1. Personal Mission and Inclusion

Zenith was born from the need to study compilers and tooling in depth, without turning every step into a struggle against the tool itself. As a project designed by and for people with ADHD and dyslexia, Zenith assumes the commitment that clarity, predictability, and welcoming are fundamental to the programming experience.

Accessibility is not a detail; it is the foundation. Zenith proves that neurodivergent minds can build ambitious technical systems when the environment reduces noise and respects the human processing rhythm.

---

## 2. Central Thesis: Readability as Infrastructure

Most modern languages sacrifice clarity in favor of extreme conciseness. Zenith inverts this logic through three principles:

*   **Low Neural Friction**: The programmer should spend energy solving the problem, not deciphering the syntax. Zenith uses explicit keywords (such as end, match, where) to ensure a predictable vertical visual flow.
*   **Visual Stability**: Mathematical symbols and operators appear only where they are semantically necessary. The grammar remains predominantly textual and readable to avoid the "letter dance" typical of dyslexia.
*   **Explicit over Implicit**: Magic behaviors are avoided. Types are named, failures are handled via Optional and Outcome, and data contracts are declared via where.

---

## 3. The Technical Trinity (v0.3.1)

Zenith's technical foundation rests upon three engineering pillars:

1.  **Native Sovereignty (Ascension)**: The compiler is written in Zenith for Zenith. This ensures total independence from external engines and allows optimizations that exploit the hardware to the maximum.
2.  **Zero-Cost Abstractions**: High-level features (Traits, Generic Enums, Static Typing) are resolved at compile-time. The generated code is as efficient as hand-written code by experts.
3.  **Safety by Default**: I/O, network, and parsing operations use the Outcome system. The compiler prevents common runtime failures from reaching production.

---

## 4. Ecosystem and Official Frameworks

The Zenith ecosystem expands the generic core through specialized tools:

### Visual Domain and Interfaces
*   **Meridian**: Declarative and reactive framework for building graphical user interfaces (GUI).
*   **Borealis**: 2D game engine focused on industrial performance, built on Prism via FFI.

### Data Intelligence and Web
*   **Atlas**: Type-safe database framework (ORM) with support for SQLite and PostgreSQL.
*   **Intersect**: MVVM web framework for the browser, allowing native Zenith in the browser.
*   **Orion**: Asynchronous HTTP server for high-performance APIs and services.

### Tooling and Intelligence
*   **Forge**: Visual editor and authoring environment for the ecosystem.
*   **Compass**: Official language server (LSP) providing code intelligence.

---

## 5. Distribution Strategy

Zenith ensures frictionless software delivery:

*   **Seed Mode (Standalone)**: Generates a single-file executable containing the VM and optimized code.
*   **Colony Mode (Bundle)**: Generates an organized directory structure (/dist) with all native dependencies managed automatically.

---
*Zenith: Where logic meets rhythm.*
