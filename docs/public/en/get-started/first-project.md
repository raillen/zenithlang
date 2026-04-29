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

Se a pasta já existe:

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

## Programa mínimo

```zt
namespace app.main

func main()
    print("Hello from Zenith")
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

## Alternativa: single-file mode

Você também pode rodar um arquivo `.zt` avulso sem projeto:

```powershell
.\zt.exe run examples\hello\src\app\main.zt
```

O compilador cria um manifesto sintetico automaticamente. O arquivo precisa
declarar `namespace` e conter uma funcao `main`.

Para projetos com varios arquivos, use o projeto:

```powershell
.\zt.exe run examples\hello\zenith.ztproj
```

## Próximo passo

Leia:

- `docs/public/learn/01-first-program.md`
- `docs/public/language/core-tour.md`
