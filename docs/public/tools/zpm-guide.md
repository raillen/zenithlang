# Zenith Package Manager (ZPM) User Guide

> Guia publico do ZPM.
> Audience: user, package-author
> Surface: public
> Status: current

## Objetivo

ZPM e o gerenciador de packages da linguagem Zenith.

No corte atual, ele existe como binario `zpm.exe` e tambem segue o modelo de integracao com o driver `zt`.

## Ajuda rapida

```powershell
.\zpm.exe help
```

## Criar projeto

```powershell
.\zpm.exe init my_project
```

Isso cria um projeto com `zenith.ztproj`.

## Adicionar dependencia

Entre na pasta do projeto:

```powershell
cd my_project
```

Adicione o package:

```powershell
..\zpm.exe add borealis@0.1.0
```

O manifesto recebe a dependencia em `[dependencies]`.

## Instalar dependencias

```powershell
..\zpm.exe install
```

Isso resolve dependencias e gera `zenith.lock`.

## Listar dependencias

```powershell
..\zpm.exe list
```

## Atualizar dependencias

```powershell
..\zpm.exe update
```

Ou atualize um package especifico:

```powershell
..\zpm.exe update borealis
```

## Remover dependencia

```powershell
..\zpm.exe remove borealis
```

## Rodar script do manifesto

```powershell
..\zpm.exe run build
```

## Publicar

```powershell
..\zpm.exe publish
```

Use `publish` como validacao antes de compartilhar um package.

## Manifesto basico

```toml
[project]
name = "my_app"
kind = "app"
version = "0.1.0"

[dependencies]
borealis = "0.1.0"
```

## Lockfile

`zenith.lock` deve ser versionado quando o projeto precisa de reproducibilidade.

## Limites do corte atual

- O ecossistema de packages ainda esta em alpha.
- Packages oficiais podem mudar antes de `1.0.0`.
- Use `zt check` depois de `zpm install` para validar o projeto real.
