# Borealis ZPM Prep v1

Data: 2026-04-22
Versao do package: `0.1.0`

## Estado

O Borealis esta preparado para virar package ZPM assim que o comando `zpm` estiver disponivel.

Ja esta alinhado:

- `project.kind = "lib"`
- `project.version = "0.1.0"`
- `lib.root_namespace = "borealis"`
- docs centrais de arquitetura, API, riscos e limites conhecidos
- changelog e guia de migracao

## Manifesto atual

Arquivo: `packages/borealis/zenith.ztproj`

Campos importantes:

- `name = "borealis"`
- `version = "0.1.0"`
- `kind = "lib"`
- `root_namespace = "borealis"`

## Modelo esperado quando ZPM chegar

Consumidor futuro:

```toml
[dependencies]
borealis = "0.1.0"
```

Durante a fase atual, o fluxo continua sendo:

- desenvolver o package localmente em `packages/borealis`
- validar com `zt check`
- manter docs e exemplos sincronizados

## Checklist de preparo

- semver inicial definido
- namespace raiz definido
- docs de onboarding presentes
- limites conhecidos publicados
- changelog inicial publicado
- guia de migracao publicado

## O que ainda depende do ZPM real

- resolver dependencias automaticamente
- instalar package em projeto consumidor
- lockfile resolvido
- publish remoto
