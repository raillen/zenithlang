# Get Started

> Primeiros passos publicos da linguagem Zenith.
> Audience: user
> Surface: public
> Status: current

## Objetivo

Criar, checar e executar um projeto Zenith local.

Use esta pagina quando quiser validar o fluxo basico sem ler a arquitetura interna.

## Requisitos

- Python 3 para compilar o repositorio.
- GCC ou Clang no `PATH` para gerar binarios nativos.
- PowerShell, bash ou shell equivalente.

## Passo 1: compilar o compilador

Na raiz do repositorio:

```powershell
python build.py
```

No Windows, tambem pode usar:

```powershell
.\build.bat
```

Resultado esperado: `zt.exe` na raiz do repositorio.

## Passo 2: ver ajuda

```powershell
.\zt.exe help
```

Comandos principais:

- `check`: valida o projeto.
- `build`: compila o projeto.
- `run`: compila e executa.
- `create`: cria um projeto novo.
- `fmt`: formata codigo.

## Passo 3: criar um projeto

```powershell
.\zt.exe create examples\hello --app
```

Se a pasta ja existir e voce quiser sobrescrever o scaffold:

```powershell
.\zt.exe create examples\hello --app --force
```

## Passo 4: checar o projeto

```powershell
.\zt.exe check examples\hello\zenith.ztproj
```

Para validar todos os arquivos do projeto:

```powershell
.\zt.exe check examples\hello\zenith.ztproj --all
```

## Passo 5: executar

```powershell
.\zt.exe run examples\hello\zenith.ztproj
```

## Fluxo recomendado

1. Edite arquivos `.zt`.
2. Rode `zt check`.
3. Rode `zt run`.
4. Use `zt build` quando precisar gerar binario.

## Proximas leituras

- Instalacao local: `docs/public/get-started/installation.md`.
- Primeiro projeto: `docs/public/get-started/first-project.md`.
- Fluxo diario: `docs/public/get-started/daily-workflow.md`.
- Guia da linguagem: `docs/public/language/README.md`.
- Referencia da linguagem: `docs/reference/language/README.md`.
- CLI: `docs/reference/cli/README.md`.
- ZPM: `docs/public/tools/zpm-guide.md`.
