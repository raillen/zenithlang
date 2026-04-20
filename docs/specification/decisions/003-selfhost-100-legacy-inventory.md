# RFC 003 - Inventario de vazamentos de legado para o corte 100% self-hosted

> Estado: historico.
> Este inventario descreve o debito da Fase 1 do corte 100% self-hosted.
> Foi superado por `decisions/004-selfhost-100-cutover.md` e pelo fechamento operacional em `../../roadmap/selfhost-pos100.md`.

- Status: aprovada para a Fase 1 do roadmap selfhost-100
- Verificado em: 2026-04-15
- Tipo: inventario / produto / bootstrap / release
- Escopo: `ztc.lua`, `tools/bootstrap.lua`, `tools/selfhost_release.lua`, `zpm`, `zman`, `ztest`

## Resumo

Esta decisao fecha a Fase 1 do roadmap `selfhost-100.md`.

Objetivo da fase: parar de discutir "onde ainda existe legado" de forma impressionista e transformar o debito restante em lista auditavel, classificada e congelada.

Resultado:

- os pontos de uso do pipeline legado agora podem ser auditados via `--strict-selfhost` em `ztc.lua`, `tools/bootstrap.lua` e `tools/selfhost_release.lua`;
- existe um inventario dinamico repetivel em `lua tools/selfhost_release.lua`;
- as superficies remanescentes foram classificadas como produto com debito, manutencao operacional e recuperacao futura.

## Evidencia Coletada

Comandos usados nesta fase:

- `lua tools/selfhost_release.lua`
- `lua ztc.lua --strict-selfhost check src/cli/zpm.zt`
- `lua ztc.lua --strict-selfhost check src/cli/zman.zt`
- `lua ztc.lua --strict-selfhost check src/cli/ztest.zt`
- `lua ztc.lua --strict-selfhost check src/cli/zman.zt`
- `lua ztc.lua --strict-selfhost check src/cli/ztest.zt`

Artefato gerado:

- `.selfhost-bootstrap/selfhost-release-report.txt`

Resultado auditado nesta data:

- bootstrap: 3 eventos de legado, todos explicitos e intencionais
- release: 5 eventos de legado, sendo 3 do bootstrap e 2 do fallback de `zpm`
- `zpm`: 2 eventos de legado e saida final ainda verde por fallback
- `zman`: 2 eventos de legado e saida final falhando
- `ztest`: 2 eventos de legado e saida final falhando

## Inventario Classificado

### 1. Superficie oficial do produto com debito ativo

Pontos congelados nesta categoria:

- `ztc.lua` ainda expone `--strict-selfhost` e mantem downgrade controlado para o pipeline legado
- builtins oficiais `zpm`, `zman` e `ztest` ainda passam por `compile_builtin_temp` com fallback para legado quando nao compilam no self-hosted

Leitura correta:

- esta e a superficie que precisa desaparecer para o produto poder se declarar 100% self-hosted
- nenhum novo fallback pode ser adicionado aqui sem atualizar esta decisao e o auditor

Estado atual dos builtins:

- `zpm`: falha em modo estrito self-hosted com erros de parser em `src/cli/zpm.zt:86` e `src/cli/zpm.zt:87`; ainda roda via fallback legado
- `zman`: falha em modo estrito self-hosted com erro de parser em `src/cli/zman.zt:85`; o legado tambem falha no mesmo ponto, logo o problema ja nao e apenas de paridade self-hosted
- `ztest`: falha em modo estrito self-hosted com erros em `src/cli/ztest.zt:28`, `src/cli/ztest.zt:29` e `src/cli/ztest.zt:135`; o legado ainda falha semanticamente em `src/cli/ztest.zt:56`

Conclusao operacional:

- Fase 2 nao e "remover fallback" de imediato
- Fase 2 e tornar `zpm`, `zman` e `ztest` compilaveis na trilha self-hosted, e no caso de `zman`/`ztest` primeiro recolocar os proprios builtins em estado verde

### 2. Superficie de manutencao operacional

Pontos congelados nesta categoria:

- `tools/bootstrap.lua` executa `--strict-selfhost check` para `src/compiler/syntax.zt` e `src/compiler/syntax_bridge.zt`
- `tools/bootstrap.lua` executa `--strict-selfhost build` para gerar o `stage1`
- `tools/selfhost_release.lua` herda esse bootstrap e, no estado atual, ainda captura o fallback de `zpm` no corpus oficial

Leitura correta:

- esta superficie nao e a face do produto para o usuario
- ela ainda participa do fluxo normal de manutencao e release
- e exatamente por isso que ela precisa ser eliminada nas Fases 3 e 4, quando o bootstrap sair do seed legado

### 3. Superficie de recuperacao a isolar

Pontos congelados nesta categoria:

- flag `--strict-selfhost` no front door
- ambiente `ZTC_ENGINE=legacy`

Leitura correta:

- essas entradas ainda existem, mas a partir desta fase deixam de ser tratadas como caminho normal do produto
- o alvo do roadmap e isolar isso como recuperacao/manutencao extraordinaria, ou remover de vez no fechamento do ciclo

## Decisoes da Fase 1

### 1. O auditor de legado vira evidencia obrigatoria do ciclo

Ferramenta oficial desta fase:

- `lua tools/selfhost_release.lua`

Uso correto:

- rodar antes de fechar Fases 2, 3, 4 e 5
- comparar se a lista de vazamentos congelada nesta decisao diminuiu
- bloquear qualquer "solucao" que apenas mova o fallback de lugar sem reduzir a superficie inventariada

### 2. A lista de vazamentos fica congelada nesta data

Lista congelada:

- downgrade do front door em `ztc.lua`
- fallback de builtins em `zpm`, `zman` e `ztest`
- bootstrap stage1 ainda sem seed self-hosted
- release ainda herdando bootstrap legado e fallback de `zpm`

Consequencia:

- qualquer novo uso de legado fora dessa lista passa a ser regressao de arquitetura, nao "ajuste pragmatico"

### 3. A ordem da Fase 2 fica definida

Ordem recomendada a partir deste inventario:

1. `zpm`: primeiro porque ja participa do gate de release e hoje ainda passa por fallback
2. `zman`: segundo porque o builtin nem sequer esta verde no legado; precisa ser recolocado em estado compilavel
3. `ztest`: terceiro porque tambem carrega quebra no legado e exige correcoes semanticas adicionais

## O que esta fase conclui

Esta fase conclui:

- o inventario auditavel dos vazamentos de legado
- a classificacao entre produto, manutencao e recuperacao
- a ordem realista de ataque da Fase 2

Esta fase nao conclui:

- remocao do fallback legado
- paridade self-hosted dos builtins
- bootstrap sem seed legado
- release 100% self-hosted

## Leitura complementar

- `../../roadmap/selfhost-100.md`
- `../../roadmap/selfhost-oficializacao.md`
- `../selfhost-abi.md`
- `.selfhost-bootstrap/selfhost-release-report.txt`
