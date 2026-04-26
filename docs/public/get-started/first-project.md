# Primeiro projeto

> Audience: user
> Status: current
> Surface: public
> Source of truth: no

## Resumo

Um projeto Zenith tem um manifesto `zenith.ztproj` e arquivos `.zt`.

Use `zt create` para gerar a estrutura inicial.

## Criar projeto

```powershell
.\zt.exe create examples\hello --app
```

Se a pasta ja existe:

```powershell
.\zt.exe create examples\hello --app --force
```

## Estrutura esperada

```text
examples/hello/
  zenith.ztproj
  src/
    app/
      main.zt
```

## Programa minimo

```zt
namespace app.main

public func main() -> int
    return 0
end
```

## Checar

```powershell
.\zt.exe check examples\hello\zenith.ztproj --all
```

## Executar

```powershell
.\zt.exe run examples\hello\zenith.ztproj
```

## Erro comum

Nao rode um arquivo `.zt` solto:

```powershell
.\zt.exe run examples\hello\src\app\main.zt
```

Use o projeto:

```powershell
.\zt.exe run examples\hello\zenith.ztproj
```

## Proximo passo

Leia:

- `docs/public/learn/01-first-program.md`
- `docs/public/language/core-tour.md`
