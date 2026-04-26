# `zpm` CLI Reference

> Surface: reference
> Status: current

## Forma geral

```text
zpm <command> [options]
```

## Commands

| Command | Use |
| --- | --- |
| `init [path|.]` | initialize a Zenith project |
| `add <pkg>[@version]` | add a dependency to `zenith.ztproj` |
| `remove <pkg>` | remove a dependency |
| `install` | install dependencies and generate `zenith.lock` |
| `update [pkg]` | update dependencies |
| `list` | list installed dependencies |
| `find <query>` | search for a package |
| `run <script>` | run a script from `zenith.ztproj` |
| `update-registry` | sync local package registry |
| `publish` | validate package publication readiness |
| `help` | show help |

## Examples

```powershell
.\zpm.exe init my_project
cd my_project
..\zpm.exe add borealis@0.1.0
..\zpm.exe install
..\zpm.exe list
..\zpm.exe publish
```

## Manifest

```toml
[project]
name = "my_app"
kind = "app"
version = "0.1.0"

[dependencies]
borealis = "0.1.0"
```

## Lockfile

Commit `zenith.lock` when the project needs reproducible dependency versions.
