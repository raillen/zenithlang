# Ascension Compiler (Zenith)

> Status atual: linha self-hosted canonica selada como produto 100% self-hosted
> Implementacao oficial hoje: `ztc.lua` em modo self-hosted estrito
> Leitura recomendada do estado real: `../roadmap/selfhost-pos100.md`

Ascension continua sendo a linha self-hosted do projeto, mas agora ela deixou de ser apenas trilha de paridade. O repositorio passou a ter uma leitura oficial precisa e operacionalmente limpa.

- frente oficial do produto: `ztc.lua` operando apenas no caminho self-hosted;
- core canonico: `src/compiler/syntax.zt`;
- bootstrap oficial: cadeia self-hosted promovida via `ztc_selfhost.lua`;
- superficie de recuperacao: `tools/ztc_legacy.lua` e `tools/bootstrap_legacy_recovery.lua`;
- superficie experimental adjacente: `syntax_bridge.zt`, `binder.zt`, `parser.zt` e `lexer.zt` fora do caminho oficial.

## O que e verdade hoje

- a linha self-hosted e a face oficial da linguagem no front door;
- a base `.zt` do compilador valida bootstrap canonico e nao carrega blocos `native lua` remanescentes no recorte oficial;
- o bootstrap `stage2/stage3` e deterministico;
- existe gate recorrente de release em `tools/selfhost_release.lua`;
- a ABI de host da linha self-hosted ficou pequena, nomeada e documentada;
- artefatos operacionais passaram a ter politica explicita;
- a auditoria oficial falha se qualquer superficie do produto ainda tocar o legado.

## O que Ascension entrega hoje

- compilador self-hosted canonico em `src/compiler/syntax.zt`;
- promocao controlada para `ztc_selfhost.lua`;
- base para verificacao de soberania em `tests/ascension/`;
- contrato curto de host ABI para sustentar parser, binder, lowering, diagnostics e codegen do core.

## O que continua fora deste corte

- eliminacao total do runtime Lua;
- promocao automatica de modulos `.zt` experimentais adjacentes;
- fim do hardening incremental de compilador.

Plano do corte 100%: `../roadmap/selfhost-100.md`
Fechamento pos-100: `../roadmap/selfhost-pos100.md`
Arquitetura canonica: `../specification/decisions/002-selfhost-architecture-cutover.md`
Corte final 100%: `../specification/decisions/004-selfhost-100-cutover.md`
Contrato de ABI: `../specification/selfhost-abi.md`
Politica de artefatos: `../specification/selfhost-artifacts.md`

## Onde isso mora no repositorio

- `src/compiler/`
- `src/compiler/syntax.zt`
- `src/compiler/syntax_bridge.zt`
- `tests/ascension/`

## Papel atual do legado em Lua

A trilha Lua segue preservada, mas sua leitura correta agora e restrita a recuperacao extraordinaria. Ela nao participa mais do caminho oficial do produto, do bootstrap normal ou do release recorrente.
