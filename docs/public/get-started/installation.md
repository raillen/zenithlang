# Instalacao local

> Audience: user
> Status: current
> Surface: public
> Source of truth: no

## Resumo

Para usar Zenith neste repositorio, compile o compilador local e rode `zt.exe`.

O fluxo atual e local-first.

## Requisitos

- Python 3.
- GCC ou Clang no `PATH`.
- PowerShell no Windows, ou shell equivalente em Linux/macOS.

## Compilar no Windows

Na raiz do repositorio:

```powershell
python build.py
```

Ou:

```powershell
.\build.bat
```

Resultado esperado:

```text
zt.exe
zpm.exe
```

## Conferir se funcionou

```powershell
.\zt.exe help
```

Depois:

```powershell
.\zt.exe check zenith.ztproj --all --ci
```

## Problemas comuns

### `zt.exe` nao apareceu

Rode novamente:

```powershell
python build.py
```

Se falhar, leia a primeira mensagem de erro real.

Mensagens de profile do PowerShell podem aparecer antes, mas nem sempre sao o
erro do build.

### GCC ou Clang nao encontrado

Instale uma toolchain C e garanta que o compilador esteja no `PATH`.

Zenith usa backend C neste corte.

## Proximo passo

Leia:

- `docs/public/get-started/first-project.md`
- `docs/public/get-started/daily-workflow.md`
