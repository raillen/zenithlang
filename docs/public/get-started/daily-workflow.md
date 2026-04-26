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

## Quando usar `check`

Use `check` quando quiser saber se o projeto esta semanticamente valido.

```powershell
.\zt.exe check zenith.ztproj --all --ci
```

## Quando usar `run`

Use `run` para compilar e executar.

```powershell
.\zt.exe run tests\behavior\simple_app\zenith.ztproj
```

## Quando usar `build`

Use `build` quando quiser gerar artefato nativo sem executar.

```powershell
.\zt.exe build examples\hello\zenith.ztproj
```

## Como ler erro

Leia nesta ordem:

1. codigo do erro;
2. local do erro;
3. mensagem principal;
4. dica de proxima acao.

Nao tente corrigir varios erros ao mesmo tempo.

Corrija o primeiro erro claro e rode o comando de novo.

## Proximo passo

Leia:

- `docs/reference/cli/zt.md`
- `docs/reference/cli/diagnostics.md`
