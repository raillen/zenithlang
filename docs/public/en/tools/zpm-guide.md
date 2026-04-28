# Zenith Package Manager (ZPM) User Guide

> ZPM public guide.
> Audience: user, package-author
> Surface: public
> Status: current

## Objective

ZPM is the Zenith language package manager.

In the current cut, it exists as binary`zpm.exe`and also follows the driver integration model`zt`.

## Quick help

```powershell
.\zpm.exe help
```

## Create project

```powershell
.\zpm.exe init my_project
```

This creates a project with`zenith.ztproj`.

## Add dependency

Enter the project folder:

```powershell
cd my_project
```

Add the package:

```powershell
..\zpm.exe add borealis@0.1.0
..\zpm.exe add json@^1.2.3
```

The manifest receives the dependency in `[dependencies]`.

Accepted versions in this cut:

- `1.2.3` for one exact version;
- `^1.2.3` for compatible updates inside the same major version;
- `~1.2.3` for patch updates inside the same minor version.

## Install dependencies

```powershell
..\zpm.exe install
..\zpm.exe install --locked
```

This resolves dependencies and generates `zenith.lock`.

Use `--locked` in CI to fail when `zenith.lock` is missing or stale.

## List dependencies

```powershell
..\zpm.exe list
```

## Update dependencies

```powershell
..\zpm.exe update
```

Or update a specific package:

```powershell
..\zpm.exe update borealis
```

## Remove dependency

```powershell
..\zpm.exe remove borealis
```

## Run manifest script

```powershell
..\zpm.exe run build
```

## Publish

```powershell
..\zpm.exe publish
```

Use`publish`as validation before sharing a package.

## Basic manifesto

```toml
[project]
name = "my_app"
kind = "app"
version = "0.1.0"

[dependencies]
borealis = "0.1.0"
```

## Lockfile

Commit `zenith.lock` when the project needs reproducibility.

## Limits of the current cut

- The package ecosystem is still in alpha.
- Official packages may change before`1.0.0`.
- Use`zt check`after`zpm install`to validate the real project.
