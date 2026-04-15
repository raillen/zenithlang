# Roadmap: Self-Hosted 100%

> Estado: historico de corte.
> O corte 100% foi fechado aqui; o fechamento operacional/editorial complementar ficou em `selfhost-pos100.md`.

Atualizado em: 2026-04-15

Status do ciclo: concluido em 2026-04-15.

Este roadmap abriu o ciclo posterior a `selfhost-oficializacao.md`.

O ciclo anterior oficializou a linha self-hosted como face do produto. O objetivo aqui foi mais estrito: eliminar a dependencia operacional da trilha legada no caminho oficial de compilacao, CLI e release.

Resultado final deste ciclo:

- `ztc.lua` opera apenas no caminho oficial self-hosted e bloqueia `--legacy` na superficie normal;
- `zpm`, `zman` e `ztest` passam em modo estrito;
- `tools/bootstrap.lua` promove a cadeia a partir de seed self-hosted;
- `tools/selfhost_release.lua` e `tools/selfhost_legacy_audit.lua` fecham o gate institucional;
- o legado ficou isolado em `tools/ztc_legacy.lua` e `tools/bootstrap_legacy_recovery.lua`.

## 1. Estado de Partida

Base herdada do ciclo anterior:

| Area | Estado inicial | Evidencia historica |
|---|---|---|
| Front door oficial | `ztc.lua` usava engine self-hosted por padrao, mas ainda em modo dual | `lua ztc.lua --selfhost --version` |
| Fallback oficial | existiam `--legacy` e fallback interno | `ztc.lua` |
| Core canonico | `src/compiler/syntax.zt` e o compilador self-hosted canonico | RFC 002 |
| Gate de release | existia e estava verde | `lua tools/selfhost_release.lua` |
| ABI de host | pequena, congelada e documentada | `tools/selfhost_abi.lua`, `docs/specification/selfhost-abi.md` |
| Builtins de CLI | ainda podiam cair para o legado | `lua ztc.lua --selfhost zpm version` |
| Bootstrap | ainda usava a trilha legada como seed operacional | `tools/bootstrap.lua` |

## 2. O Que Significa 100% Self-Hosted

Este roadmap so estaria concluido quando todos os itens abaixo fossem verdade ao mesmo tempo:

- o front door oficial executar `check`, `build`, `run`, `zpm`, `zman` e `ztest` sem fallback legado no caminho oficial;
- `tools/selfhost_release.lua` falhar se qualquer etapa oficial usar pipeline legado;
- o release oficial puder ser promovido a partir de artefato self-hosted anterior, sem depender do compilador legado como etapa normal de produto;
- `--legacy` deixar de existir como superficie normal de usuario e, se ainda for mantido, ficar isolado como ferramenta de recuperacao/manutencao;
- a documentacao principal parar de tratar a trilha legada como opcao operacional valida do produto atual.

## 3. O Que 100% Self-Hosted Nao Significa

Este roadmap nao significa:

- remover o backend Lua como alvo de codigo gerado;
- remover o runtime Lua dos programas compilados;
- promover automaticamente `binder.zt`, `parser.zt` e `lexer.zt` experimentais;
- reabrir a topologia canonica de `src/compiler/syntax.zt`.

O alvo aqui e o compilador/produto ser self-hosted de ponta a ponta no fluxo oficial, nao trocar o backend de execucao da linguagem.

## 4. Lacunas Reais do Inicio do Ciclo

As lacunas objetivas no inicio eram:

- builtins de CLI ainda nao compilavam integralmente na trilha self-hosted;
- o front door ainda aceitava queda controlada para o legado;
- o bootstrap oficial ainda partia da trilha legada como seed normal;
- o gate de release ainda aceitava fallback de builtin como limite deliberado do corte anterior;
- a superficie de recuperacao e a superficie oficial ainda conviviam no mesmo entrypoint.

Todas essas lacunas foram fechadas neste ciclo.

## 5. Frentes de Trabalho

### Frente 1. Paridade completa de CLI

Objetivo: fazer `zpm`, `zman` e `ztest` rodarem pelo compilador self-hosted sem downgrade.

Criterio de aceite:

- smoke de `version/help` e fluxos minimos dos builtins passam em modo estritamente self-hosted;
- nenhum builtin oficial depende de fallback legado no front door.

Status final: concluida.

### Frente 2. Front door estrito

Objetivo: transformar a trilha self-hosted em caminho unico do produto.

Criterio de aceite:

- `ztc.lua` falha em vez de cair para o legado no caminho oficial;
- qualquer compatibilidade residual fica fora da superficie normal de usuario.

Status final: concluida.

### Frente 3. Bootstrap sem legado operacional

Objetivo: tirar o compilador legado do fluxo normal de promocao.

Criterio de aceite:

- promocao de release parte de artefato self-hosted anterior ou seed self-hosted controlada;
- o legado, se ainda existir, fica restrito a recuperacao extraordinaria.

Status final: concluida.

### Frente 4. Gate de release 100%

Objetivo: impedir regressao institucional para o legado.

Criterio de aceite:

- `tools/selfhost_release.lua` detecta e bloqueia qualquer uso de caminho legado no fluxo oficial;
- release so passa quando toda a superficie oficial esta verde em modo estrito.

Status final: concluida.

## 6. Fases 1 a 5

### Fase 1. Inventariar e bloquear vazamentos de legado

Status: concluida em 2026-04-15.

Objetivo: mapear onde o legado ainda entrava no fluxo oficial e transformar isso em lista fechada de trabalho.

Entregaveis:

- inventario de todos os pontos de fallback em `ztc.lua`, scripts de bootstrap e gate;
- modo de auditoria que identificava uso do pipeline legado durante release e smoke de CLI;
- classificacao do que era superficie oficial, superficie de manutencao e superficie de recuperacao.

Entregue nesta fase:

- `ztc.lua`, `tools/bootstrap.lua` e `tools/selfhost_release.lua` passaram a aceitar ou propagar `--audit-legacy <path>`;
- `tools/selfhost_legacy_audit.lua` passou a gerar o inventario dinamico em `.selfhost-bootstrap/selfhost-legacy-audit.txt`;
- RFC 003 registrou a classificacao entre superficie oficial com debito, manutencao operacional e recuperacao futura;
- a ordem de ataque da Fase 2 ficou congelada em `zpm -> zman -> ztest`.

### Fase 2. Fechar a paridade self-hosted dos builtins

Status: concluida em 2026-04-15.

Objetivo: promover a superficie oficial de CLI para modo estrito.

Entregaveis:

- correcoes de parser/semantica/codegen que impediam `zpm`, `zman` e `ztest` de compilar no self-hosted;
- corpus minimo de builtin em modo estrito;
- remocao do fallback legado do caminho oficial dos builtins.

Criterio de aceite:

- `lua ztc.lua --strict-selfhost zpm version`
- `lua ztc.lua --strict-selfhost zman help`
- `lua ztc.lua --strict-selfhost ztest --help`

Todos passam sem compilar nada pela trilha legada.

Entregue nesta fase:

- `zpm.zt` foi reescrito para um recorte compativel com a trilha self-hosted e passou em `version/help/doctor`;
- `zman.zt` foi reduzido a um manual textual deterministicamente compilavel, com `help/list/show`;
- `ztest.zt` foi refeito com descoberta simples de suites, filtro `--grep`, suporte a `.zt` e `.lua`, e `--help` verde;
- o emitter self-hosted foi corrigido para emitir chamadas de modulo importado com `.` em vez de `:`.

### Fase 3. Endurecer o front door oficial

Status: concluida em 2026-04-15.

Objetivo: trocar o front door de "self-hosted com rede de seguranca" para "self-hosted ou falha".

Entregaveis:

- `tools/selfhost_release.lua` rodando em modo estrito;
- `ztc.lua` sem downgrade legado na superficie oficial;
- caminho legado movido para ferramenta separada de manutencao, fora do uso normal do produto.

Criterio de aceite:

- release oficial nao depende mais de `--legacy`;
- usuario normal nao encontra mais o legado como opcao de fluxo principal.

Entregue nesta fase:

- `ztc.lua` nao faz mais downgrade para a trilha legada no caminho oficial;
- `--legacy` foi removido da superficie normal e passou a falhar com orientacao de recuperacao;
- o legado ficou isolado em `tools/ztc_legacy.lua`.

### Fase 4. Reestruturar o bootstrap para seed self-hosted

Status: concluida em 2026-04-15.

Objetivo: fazer a promocao oficial partir de cadeia self-hosted, nao do compilador legado.

Entregaveis:

- seed oficial baseada em artefato self-hosted promovido anteriormente;
- verificacao de recompilacao/self-rebuild com determinismo;
- plano de recuperacao separado do fluxo normal para o caso de seed quebrada.

Criterio de aceite:

- o release de rotina nao executa compilador legado;
- o bootstrap de produto roda em cadeia self-hosted controlada.

Entregue nesta fase:

- `tools/bootstrap.lua` agora verifica e promove por `--strict-selfhost`;
- a seed oficial de promocao e o artefato self-hosted promovido anteriormente;
- a recuperacao extraordinaria ficou separada em `tools/bootstrap_legacy_recovery.lua`.

### Fase 5. Declarar e selar o corte 100%

Status: concluida em 2026-04-15.

Objetivo: concluir o corte tecnico e institucional.

Entregaveis:

- docs principais atualizados para dizer que o produto e 100% self-hosted;
- gate final bloqueando qualquer regressao para fallback legado;
- reposicionamento definitivo do legado como ferramenta de arquivo/recuperacao, ou remocao do repositorio se essa for a decisao.

Criterio de aceite:

- a linguagem pode se declarar 100% self-hosted de forma defensavel;
- o caminho oficial inteiro passa em modo estrito;
- o legado nao participa mais do release normal.

Entregue nesta fase:

- docs principais atualizados para declarar o corte 100% self-hosted;
- `tools/selfhost_abi.lua` e `docs/specification/selfhost-abi.md` foram alinhados ao corpus estrito;
- `tools/selfhost_legacy_audit.lua` passou a falhar se qualquer superficie oficial ainda tocar o legado;
- RFC 004 registrou o corte final entre superficie oficial e recuperacao.

## 7. Validacao Minima para Encerramento

O corte 100% self-hosted encerrou com pelo menos este recorte verde:

- `lua ztc.lua --strict-selfhost --version`
- `lua ztc.lua --strict-selfhost check src/compiler/syntax.zt`
- `lua ztc.lua --strict-selfhost build demo.zt out.lua`
- `lua ztc.lua --strict-selfhost run tests/stdlib/test_optional.zt`
- `lua ztc.lua --strict-selfhost zpm version`
- `lua ztc.lua --strict-selfhost zman help`
- `lua ztc.lua --strict-selfhost ztest --help`
- `lua tools/selfhost_release.lua`
- `lua tools/selfhost_legacy_audit.lua`
- bootstrap/promocao sem executar pipeline legado no fluxo oficial
- bateria principal de parser, semantic e codegen verde

Validacao executada no fechamento:

- `lua tools/bootstrap.lua --promote --target ztc_selfhost.lua`
- `lua tools/selfhost_release.lua`
- `lua tools/selfhost_legacy_audit.lua`
- `lua ztc.lua --strict-selfhost zpm version`
- `lua ztc.lua --strict-selfhost zman help`
- `lua ztc.lua --strict-selfhost ztest --help`
- `lua ztc.lua --strict-selfhost ztest --grep parser_tests/test_parser.lua`
- `lua ztc.lua --strict-selfhost ztest --grep tests/stdlib/test_optional.zt`

## 8. Nao Objetivos Deste Ciclo

Este roadmap nao reabre:

- migracao massiva de `null` para `Optional`;
- promocao de modulos experimentais adjacentes ao core;
- troca do backend Lua por outro alvo;
- refatoracao estetica ampla do compilador sem impacto no corte 100%.

## 9. Fechamento

Quando este roadmap terminou, a pergunta deixou de ser "o produto se apresenta como self-hosted?" e passou a ser "o produto ainda precisa do legado para existir?". A resposta no fechamento ficou: nao.

Marco final:

- `ztc.lua` opera apenas em modo self-hosted no caminho oficial;
- builtins oficiais passam sem downgrade;
- release e bootstrap normais nao executam compilador legado;
- o legado ficou fora do caminho oficial de produto.
