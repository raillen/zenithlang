# Fluxo diario

> Audience: user
> Status: current
> Surface: public
> Source of truth: no

## Resumo

O fluxo normal e simples:

1. editar;
2. formatar;
3. checar;
4. executar.

## Comandos principais

```powershell
.\zt.exe fmt examples\hello\zenith.ztproj
.\zt.exe check examples\hello\zenith.ztproj --all
.\zt.exe run examples\hello\zenith.ztproj
```

## Quando usar single-file mode

Para scripts rápidos, prototipacao ou arquivos avulsos, rode diretamente:

```powershell
.\zt.exe run hello.zt
.\zt.exe check hello.zt
```

Não precisa de `zenith.ztproj` nem de estrutura de pastas.

## Quando usar `check`

Use `check` quando quiser saber se o projeto esta semanticamente válido.

```powershell
.\zt.exe check zenith.ztproj --all --ci
.\zt.exe check hello.zt
```

## Quando usar `run`

Use `run` para compilar e executar.

```powershell
.\zt.exe run tests\behavior\simple_app\zenith.ztproj
.\zt.exe run hello.zt
```

## Quando usar `build`

Use `build` quando quiser gerar artefato nativo sem executar.

```powershell
.\zt.exe build examples\hello\zenith.ztproj
```

## Como ler erro

Leia nesta ordem:

1. código do erro;
2. local do erro;
3. mensagem principal;
4. dica de próxima ação.

Não tente corrigir varios erros ao mesmo tempo.

Corrija o primeiro erro claro e rode o comando de novo.

## Próximo passo

Leia:

- `docs/reference/cli/zt.md`
- `docs/reference/cli/diagnostics.md`
