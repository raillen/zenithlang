# RFC 002 - Arquitetura canonica self-hosted e criterio de corte do front door

> Estado: base arquitetural vigente.
> Esta decisao foi implementada pelo corte de `004-selfhost-100-cutover.md` e consolidada operacionalmente em `../../roadmap/selfhost-pos100.md`.

- Status: aprovada para oficializacao self-hosted
- Verificado em: 2026-04-15
- Tipo: arquitetura / release / bootstrap
- Escopo: `src/compiler/syntax.zt`, `ztc.lua`, runtime host ABI, docs e promocao de CLI

## Resumo

Esta decisao fecha a ambiguidade arquitetural da linha self-hosted antes da troca do front door oficial.

Direcao adotada:

- `src/compiler/syntax.zt` continua sendo o compilador self-hosted canonico.
- `src/compiler/syntax_bridge.zt` continua congelado como legado parseavel.
- `src/semantic/binding/binder.zt`, `src/syntax/parser/parser.zt` e `src/syntax/lexer/lexer.zt` ficam classificados como experimentais e fora do caminho oficial ate promocao explicita.
- `ztc.lua` continua como bootstrap/fallback durante a oficializacao, mas deixa de ser tratado como direcao arquitetural da linguagem.

Objetivo principal: garantir uma historia unica e defensavel de compilador antes de promover a linha self-hosted como face oficial do produto.

## Contexto

O repositorio saiu da Trilha B com o core canonico validado:

- `src/compiler/syntax.zt` passa em `check`
- o bootstrap `stage2/stage3` e deterministico
- `src/compiler` e `src/stdlib` ficaram sem `native lua` em `.zt`

O proximo risco nao era tecnico de bootstrap; era editorial e arquitetural:

- ainda existe `ztc.lua` como front door oficial
- existem modulos `.zt` paralelos com TODOs reais
- existia espaco para duas leituras conflitantes da arquitetura self-hosted

Sem decidir isso primeiro, qualquer corte de CLI ou release ficaria sem criterio estavel.

## Decisoes

### 1. O compilador self-hosted canonico permanece monolitico em `src/compiler/syntax.zt`

Esta e a unidade oficial de linguagem para o ciclo de oficializacao.

Consequencias:

- o bootstrap continua orientado a `src/compiler/syntax.zt`
- comparacoes de paridade e gates de release partem desse arquivo
- qualquer extracao modular futura so vale depois de promocao explicita e revalidacao do bootstrap

Racional:

- este e o unico caminho `.zt` hoje validado de ponta a ponta
- reabrir a topologia interna do compilador antes do corte do front door aumenta risco sem entregar valor imediato

### 2. Modulos `.zt` adjacentes ficam congelados como experimentais, nao como caminho oficial

Arquivos classificados assim nesta decisao:

- `src/semantic/binding/binder.zt`
- `src/syntax/parser/parser.zt`
- `src/syntax/lexer/lexer.zt`

Leitura correta:

- podem servir como laboratorio, referencia ou base de extracao futura
- nao definem o estado oficial da linguagem
- nao entram no criterio de pronto do front door por default

Para promover qualquer um deles ao caminho oficial, o minimo exigido passa a ser:

- remover TODOs que bloqueiam uso real
- amarrar testes de sanidade no pipeline
- demonstrar que a promocao nao reabre o bootstrap canonico
- atualizar docs canonicos e esta decisao

### 3. `syntax_bridge.zt` continua apenas como legado parseavel

Esta decisao nao reabre o bridge.

Leitura correta:

- `syntax_bridge.zt` nao disputa canon com `syntax.zt`
- o arquivo existe para compatibilidade editorial/historica
- qualquer referencia ao self-hosted oficial deve apontar para `src/compiler/syntax.zt`

### 4. O corte do front door oficial passa a ter pre-requisitos objetivos

`ztc.lua` so deixa de ser a porta principal quando todos os itens abaixo estiverem verdes no recorte definido:

1. existir um caminho documentado e repetivel para `check`, `build` e `run` puxado pela linha self-hosted
2. o corpus minimo de promocao estiver verde pela trilha escolhida
3. a ABI de host self-hosted estiver inventariada e congelada como contrato curto
4. docs principais e CLI de transicao estiverem alinhados ao novo entrypoint
5. a trilha Lua estiver reposicionada como bootstrap/fallback, nao como identidade principal

Enquanto isso nao acontecer:

- `ztc.lua` segue ativo
- mas sua leitura correta e bootstrap/fallback em transicao

### 5. O corpus minimo de promocao fica definido nesta fase

O corte do front door nao sera decidido por impressao subjetiva. O corpus minimo passa a ser:

- `lua ztc.lua check src\\compiler\\syntax.zt`
- `lua tools\\bootstrap.lua`
- `lua tests\\parser_tests\\test_parser.lua`
- `lua tests\\semantic_tests\\test_binder.lua`
- `lua tests\\semantic_tests\\test_semantic.lua`
- `lua tests\\codegen_tests\\test_codegen.lua`
- `lua tests\\codegen_tests\\test_match_lowering.lua`
- `lua tests\\codegen_tests\\test_option_result_codegen.lua`
- `lua ztc.lua run tests\\stdlib\\test_optional.zt`
- build da demo principal

Promocao de release self-hosted depois desta fase deve partir desse corpus e expandir, nunca encolher.

### 6. A ABI inicial de host self-hosted fica congelada como superficie minima aceita

Entradas aceitas nesta fase:

- `zt.selfhost.has_slot`
- `zt.selfhost.text_len`
- `zt.selfhost.text_slice`
- `zt.selfhost.text_replace`
- `zt.selfhost.value_is_present`
- `zt.selfhost.empty_value`
- `zt.selfhost.read_module_source`
- `zt.selfhost.fold_number_binary`
- `zt.selfhost.host_os`
- `zt.selfhost.compile_result_text`
- `zt.selfhost.run_cli`

Regras a partir daqui:

- novas entradas devem ser justificadas como necessidade de host, nao como atalho de implementacao
- cada entrada nova precisa nascer no runtime, no binding `extern` e na documentacao de arquitetura
- a tendencia desejada e manter essa superficie pequena e nomeada

## O que esta fase conclui

Esta fase conclui:

- a arquitetura self-hosted canonica
- a classificacao editorial dos modulos `.zt` adjacentes
- o corpus minimo de promocao
- a ABI inicial de host para a trilha self-hosted

Esta fase nao conclui:

- a troca do front door oficial
- a promocao dos modulos experimentais
- a eliminacao do bootstrap em Lua
- a oficializacao final de release

## Impacto pratico

Depois desta decisao:

- novas discussoes sobre self-hosted partem de `src/compiler/syntax.zt`
- `binder.zt`, `parser.zt` e `lexer.zt` deixam de ficar em limbo editorial
- Fase 2 pode atacar CLI e docs sem reabrir a arquitetura
- Fases 3-5 passam a ter criterio objetivo de promocao

## Leitura complementar

- `../current-core.md`
- `../../roadmap/selfhost-oficializacao.md`
- `../../roadmap/selfhost-consolidacao.md`
- `../../roadmap/selfhost-100.md`
- `../../roadmap/selfhost-pos100.md`
