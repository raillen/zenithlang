# Get Started

> Primeiros passos publicos da linguagem Zenith.
> Audience: user
> Surface: public
> Status: current

## Objetivo

Executar um arquivo `.zt` avulso ou criar, checar e executar um projeto Zenith local.

Use esta página quando quiser validar o fluxo básico sem ler a arquitetura interna.

## Requisitos

- Python 3 para compilar o repositorio.
- GCC ou Clang no `PATH` para gerar binarios nativos.
- PowerShell, bash ou shell equivalente.

## Passo 1: compilar o compilador

Na raiz do repositorio:

```powershell
python build.py
```

No Windows, também pode usar:

```powershell
.\build.bat
```

Resultado esperado: `zt.exe` na raiz do repositorio.

## Passo 2: modo rápido (arquivo avulso)

Crie um arquivo `hello.zt`:

```zt
namespace script

import std.io as io

func main() -> result<void, core.Error>
    io.write("Hello from Zenith\n")?
    return success()
end
```

Rode diretamente:

```powershell
.\zt.exe run hello.zt
```

Não precisa de projeto nem de `zenith.ztproj`. Tambem funciona com `check` e `build`.

## Passo 3: ver ajuda

```powershell
.\zt.exe help
```

Comandos principais:

- `check`: válida projeto ou arquivo `.zt`.
- `build`: compila projeto ou arquivo `.zt`.
- `run`: compila e executa projeto ou arquivo `.zt`.
- `create`: cria um projeto novo.
- `fmt`: formata código.

## Passo 4: criar um projeto (modo completo)

```powershell
.\zt.exe create examples\hello --app
```

Se a pasta já existir e você quiser sobrescrever o scaffold:

```powershell
.\zt.exe create examples\hello --app --force
```

## Passo 5: checar o projeto

```powershell
.\zt.exe check examples\hello\zenith.ztproj
```

Para validar todos os arquivos do projeto:

```powershell
.\zt.exe check examples\hello\zenith.ztproj --all
```

## Passo 6: executar

```powershell
.\zt.exe run examples\hello\zenith.ztproj
```

## Fluxo recomendado

1. Edite arquivos `.zt`.
2. Rode `zt check`.
3. Rode `zt run`.
4. Use `zt build` quando precisar gerar binario.

## Próximas leituras

- Instalacao local: `docs/public/get-started/installation.md`.
- Primeiro projeto: `docs/public/get-started/first-project.md`.
- Fluxo diario: `docs/public/get-started/daily-workflow.md`.
- Guia da linguagem: `docs/public/language/README.md`.
- Referência da linguagem: `docs/reference/language/README.md`.
- CLI: `docs/reference/cli/README.md`.
- ZPM: `docs/public/tools/zpm-guide.md`.
