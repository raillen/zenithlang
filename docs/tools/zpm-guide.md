# Zenith Package Manager (ZPM) User Guide

ZPM is the official package and dependency manager for the Zenith language. It is currently integrated into the main `zt` driver.

## Getting Started

### 1. Initialize a Project
Create a new Zenith project with a manifest:
```bash
zpm init my_project
```
This creates `my_project/zenith.ztproj`.

### 2. Add Dependencies
Add a package to your project:
```bash
zpm add borealis@0.1.0 my_project
```
This updates the `[dependencies]` section in your `zenith.ztproj`.

### 3. Install and Lock
Resolve dependencies and generate the lockfile:
```bash
zpm install my_project
```
This creates `zenith.lock`, which ensures version reproducibility across machines.

### 4. Publish
Validate your package before sharing:
```bash
zpm publish my_project
```
This runs a full suite of type checks and documentation audits.

## Manifest Layout
A typical `zenith.ztproj` managed by ZPM:

```toml
[project]
name = "my_app"
kind = "app"
version = "0.1.0"

[dependencies]
borealis = "0.1.0"

[dev_dependencies]
std.test = "0.1.0"
```

## Lockfile Usage
The `zenith.lock` file should be committed to version control. It specifies the exact sources and versions used in your project.
