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
```

The manifest receives the dependency on`[dependencies]`.

## Install dependencies

```powershell
..\zpm.exe install
```

This resolves dependencies and generates`zenith.lock`.

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

`zenith.lock`It must be versioned when the project needs reproducibility.

## Limits of the current cut

- The package ecosystem is still in alpha.
- Official packages may change before`1.0.0`.
- Use`zt check`after`zpm install`to validate the real project.