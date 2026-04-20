# RFC 004: Self-Hosted 100% Cutover

> Estado: vigente para o corte 100% self-hosted.
> O fechamento operacional/editorial complementar deste corte ficou em `../../roadmap/selfhost-pos100.md`.

Status: aceita
Data: 2026-04-15

## Contexto

O ciclo de oficializacao anterior promoveu a linha self-hosted como face do produto, mas ainda aceitava fallback legado no front door, nos builtins e no bootstrap normal. Esse arranjo era operacionalmente util, mas impedia declarar o produto como 100% self-hosted.

## Decisao

O corte 100% self-hosted passa a ser:

- `ztc.lua` opera apenas no caminho oficial self-hosted;
- `--strict-selfhost` sai da superficie normal de usuario e passa a falhar no front door oficial;
- a trilha legada fica isolada em `ztc.lua` exclusivamente para recuperacao/manutencao;
- o bootstrap oficial passa a usar seed self-hosted promovida anteriormente;
- a recuperacao extraordinaria de seed fica separada em `tools/bootstrap.lua`;
- o gate institucional falha se qualquer superficie oficial ainda tocar o legado.

## Consequencias

### Positivas

- o produto pode se declarar 100% self-hosted de forma defensavel;
- release, bootstrap e CLI oficial passam pelo mesmo caminho estrito;
- qualquer regressao para o legado vira falha objetiva de gate, nao comportamento implicito.

### Deliberadas

- o runtime Lua continua existindo como backend de execucao;
- o legado nao foi removido do repositorio, apenas isolado;
- modulos experimentais adjacentes ao core continuam fora do caminho oficial.

## Evidencias operacionais do corte

- `lua tools/bootstrap.lua --promote --target ztc_selfhost.lua`
- `lua tools/selfhost_release.lua`
- `lua tools/selfhost_release.lua`
- `lua ztc.lua --strict-selfhost zpm version`
- `lua ztc.lua --strict-selfhost zman help`
- `lua ztc.lua --strict-selfhost ztest --help`

## Leitura complementar

- `current-core.md`
- `selfhost-abi.md`
- `../roadmap/selfhost-100.md`
- `002-selfhost-architecture-cutover.md`
