# `zt` CLI Reference

> Surface: reference
> Status: current

## Forma geral

```text
zt <command> [input] [options]
```

## Comandos principais

| Comando | Uso |
| --- | --- |
| `help` | mostra ajuda geral ou de comando |
| `check` | valida projeto ou manifesto |
| `build` | compila projeto |
| `run` | compila e executa projeto |
| `create` | cria scaffold de projeto |
| `test` | executa testes |
| `fmt` | formata codigo |
| `doc check` | valida documentacao |
| `doc show` | mostra documentacao |
| `summary` | resume projeto |
| `resume` | retoma fluxo operacional |
| `perf` | comandos de performance |

## Ajuda

```powershell
.\zt.exe help
.\zt.exe help check
.\zt.exe help build
.\zt.exe help run
.\zt.exe help create
```

## Check

```powershell
.\zt.exe check zenith.ztproj
.\zt.exe check zenith.ztproj --all
.\zt.exe check zenith.ztproj --ci
.\zt.exe check zenith.ztproj --focus packages\borealis
```

Options accepted by help:

```text
--ci --profile <level> --all --focus <path> --since <git-ref> --lang <lang>
```

## Build

```powershell
.\zt.exe build zenith.ztproj
.\zt.exe build zenith.ztproj -o build\app.exe
.\zt.exe build zenith.ztproj --native-raw
```

## Run

```powershell
.\zt.exe run zenith.ztproj
.\zt.exe run zenith.ztproj --native-raw
```

## Create

```powershell
.\zt.exe create examples\hello --app
.\zt.exe create packages\my_lib --lib
.\zt.exe create examples\hello --app --force
```

## Compatibility Commands

The driver still exposes compatibility commands for older workflows:

```text
project-info verify emit-c build <file.zir> doc-check parse
```

Prefer the main commands for new documentation.
