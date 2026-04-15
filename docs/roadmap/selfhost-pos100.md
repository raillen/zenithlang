# Roadmap: Self-Hosted Pos-100

Atualizado em: 2026-04-15

Status do ciclo: concluido em 2026-04-15.

Este roadmap fecha o ciclo posterior a `selfhost-100.md`.

O corte tecnico de produto 100% self-hosted ja estava fechado. O objetivo aqui foi remover o residuo que ainda fazia o repositorio parecer transitorio: artefatos sem politica clara, leitura institucional ambigua e builtins oficiais ainda no menor recorte seguro.

## 1. Resultado Final

O ciclo terminou com tres entregas objetivas:

- politica de artefatos explicitada em `docs/specification/selfhost-artifacts.md`;
- fluxo operacional isolado em `.selfhost-artifacts/` e `.ztc-tmp/`, com limpeza oficial em `lua tools/selfhost_cleanup.lua`;
- builtins oficiais menos provisorios, sem reabrir legado no caminho do produto.

Estado final deste ciclo:

| Frente | Estado final | Evidencia |
|---|---|---|
| Artefatos | outputs temporarios isolados e ignorados | `.gitignore`, `tools/bootstrap.lua`, `tools/selfhost_release.lua`, `tools/selfhost_legacy_audit.lua` |
| Documentacao historica | docs antigos marcados como historicos/superados | roadmaps e RFCs do ciclo anterior |
| CLI oficial | `zpm`, `zman` e `ztest` com UX mais defendivel em modo estrito | smokes finais deste roadmap |

## 2. O Que Foi Fechado

### Frente 1. Higiene de artefatos

Entregue:

- `.selfhost-artifacts/bootstrap`, `.selfhost-artifacts/release` e `.selfhost-artifacts/audit` como destinos padrao dos fluxos oficiais;
- `.ztc-tmp/` como area unica para temporarios de compilacao e execucao;
- `.gitignore` alinhado com o fluxo atual;
- limpeza oficial via `lua tools/selfhost_cleanup.lua`, com opcao `--legacy-scratch` para sobras ignoradas em `.selfhost-bootstrap/`.

### Frente 2. Leitura institucional correta

Entregue:

- `selfhost-pos100.md` como fechamento do residuo posterior ao corte 100%;
- docs correntes apontando para o estado atual e para a politica de artefatos;
- docs historicos do ciclo anterior marcados explicitamente como leitura historica, nao como estado vigente.

### Frente 3. Polimento dos builtins oficiais

Entregue:

- `zpm` com `help`, `doctor`, `init`, `scripts`, `run` e `clean` em recorte de uso real;
- `zpm run` funcionando fora da raiz do repositorio por meio de `ZTC_FRONTDOOR_PATH` exportado pelo front door;
- `zpm clean` removendo artefatos locais seguros em `.ztc-tmp/` e `.selfhost-artifacts/`;
- `zman` com `help`, `list`, `show <topico>` e fonte exibida;
- `ztest` com `--help`, `--version`, `--list`, `--grep` e `--report`, emitindo relatorio sob `.selfhost-artifacts/ztest/`.

## 3. Fases 1 a 5

### Fase 1. Congelar politica de artefatos

Status: concluida.

Aceite fechado:

- o repositorio agora diferencia artefato promovido, temporario e historico;
- o fluxo oficial parou de espalhar temporarios pela raiz funcional.

### Fase 2. Limpar e isolar o fluxo operacional

Status: concluida.

Aceite fechado:

- bootstrap, release e auditoria escrevem em diretorios previsiveis;
- existe ferramenta oficial de limpeza;
- os comandos do produto deixam estado local limpavel de forma deterministica.

### Fase 3. Marcar a historia como historia

Status: concluida.

Aceite fechado:

- um leitor novo encontra primeiro os docs correntes;
- roadmaps e RFCs do estado anterior nao competem mais com a leitura oficial do produto atual.

### Fase 4. Endurecer UX dos builtins oficiais

Status: concluida.

Aceite fechado:

- `zpm` saiu do recorte minimo de sobrevivencia e cobre fluxo real de projeto local;
- `zman` e `ztest` continuam estritos no self-hosted com comandos uteis de produto;
- o hardening nao recolocou fallback legado no front door.

### Fase 5. Selar o pos-100

Status: concluida.

Aceite fechado:

- o residuo tecnico/editorial derivado de `selfhost-100` foi rebaixado para manutencao incremental normal.

## 4. Validacao Final

Recorte executado neste fechamento:

- `lua tools/bootstrap.lua --promote --target ztc_selfhost.lua`
- `lua tools/selfhost_release.lua`
- `lua tools/selfhost_legacy_audit.lua`
- `lua ztc.lua --strict-selfhost zpm help`
- `lua ztc.lua --strict-selfhost zpm doctor`
- smoke local sequencial:
  - `lua ..\..\ztc.lua --strict-selfhost zpm init`
  - `lua ..\..\ztc.lua --strict-selfhost zpm scripts`
  - `lua ..\..\ztc.lua --strict-selfhost zpm run hello`
  - `lua ..\..\ztc.lua --strict-selfhost zpm clean`
- `lua ztc.lua --strict-selfhost zman list`
- `lua ztc.lua --strict-selfhost zman show std.core`
- `lua ztc.lua --strict-selfhost ztest --help`
- `lua ztc.lua --strict-selfhost ztest --grep parser_tests/test_parser.lua`
- `lua ztc.lua --strict-selfhost ztest --report --grep parser_tests/test_parser.lua`

## 5. Leitura Correta Depois Deste Ciclo

A leitura correta do repositorio agora e:

- `selfhost-100.md` fecha o corte do produto 100% self-hosted;
- `selfhost-pos100.md` fecha o residuo operacional/editorial posterior;
- `current-core.md`, `current.md` e `ascension-zenith.md` descrevem o estado corrente;
- o legado continua isolado apenas para recuperacao extraordinaria;
- o que sobra agora nao e mais transicao de arquitetura, e sim manutencao incremental normal do compilador/produto.
