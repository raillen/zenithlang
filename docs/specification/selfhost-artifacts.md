# Especificacao: Self-Hosted Artifact Policy

Atualizado em: 2026-04-15

Este documento define o destino de cada artefato gerado pelos fluxos de bootstrap, release, auditoria e smoke do produto self-hosted.

## 1. Objetivo

A politica existe para evitar tres classes de ruido:

- artefato temporario aparecendo como se fosse fonte do produto;
- fluxo oficial sujando a raiz do repositorio sem necessidade;
- leitor novo confundindo artefato promovido, artefato historico e artefato descartavel.

## 2. Classes de Artefato

### Artefato versionado canonico

Arquivos versionados que continuam fazendo parte do repositorio:

- `src/compiler/syntax.lua`: artefato transicional ainda preservado no repo;
- snapshots historicos em `.selfhost-bootstrap/syntax_stage*.lua` e `.selfhost-bootstrap/summary.txt`.

Regra:

- continuam versionados;
- nao sao o destino do fluxo oficial atual;
- a superficie oficial nao depende mais deles para funcionamento normal.

### Artefato promovido local

Artefatos locais que podem representar um estado promovido do compilador no ambiente do desenvolvedor:

- `ztc_selfhost.lua`
- `src/compiler/syntax.lua.map`

Regra:

- ficam fora do fluxo temporario;
- sao ignorados por padrao no worktree;
- entram em commit apenas por decisao explicita.

### Artefato operacional temporario

Artefatos produzidos por fluxos oficiais recorrentes:

- `.selfhost-artifacts/bootstrap/`
- `.selfhost-artifacts/release/`
- `.selfhost-artifacts/audit/`
- `.selfhost-artifacts/ztest/`
- `.ztc-tmp/`

Regra:

- sao descartaveis;
- devem ficar fora da raiz funcional do repositorio;
- entram em `.gitignore`;
- podem ser removidos por `lua tools/selfhost_cleanup.lua`.

### Artefato historico legado

Artefatos antigos em `.selfhost-bootstrap/` que existiam antes da politica atual.

Regra:

- nao definem mais o fluxo oficial;
- podem permanecer como registro historico versionado ou resquicio local;
- scratch novo nessa pasta nao deve ser incentivado;
- limpeza opcional de sobras ignoradas pode ser feita com `lua tools/selfhost_cleanup.lua --strict-selfhost-scratch`.

## 3. Fluxos Oficiais

### Bootstrap

- script: `tools/bootstrap.lua`
- output-dir padrao: `.selfhost-artifacts/bootstrap`

### Release

- script: `tools/selfhost_release.lua`
- output-dir padrao do demo temporario: `.selfhost-artifacts/release`

### Auditoria

- script: `tools/selfhost_release.lua`
- output-dir padrao: `.selfhost-artifacts/audit`

### Front door

- script: `ztc.lua`
- temporarios internos: `.ztc-tmp/run` e `.ztc-tmp/builtin`
- candidatos operacionais oficiais: `ztc_selfhost.lua`, `.selfhost-artifacts/bootstrap/syntax_stage2.lua` e `src/compiler/syntax.lua`

### CLI local de projeto

- `zpm clean` remove `.ztc-tmp/` e `.selfhost-artifacts/` do diretorio atual;
- `ztest --report` escreve sob `.selfhost-artifacts/ztest/`.

## 4. Regra de Commit

Nao entram por padrao em commit:

- `.selfhost-artifacts/`
- `.ztc-tmp/`
- `ztc_selfhost.lua`
- `src/compiler/syntax.lua.map`
- scratch ignorado em `.selfhost-bootstrap/`

Podem entrar em commit apenas por decisao explicita:

- mudancas reais em fonte;
- documentacao;
- artefato promovido que o repositorio decidir preservar.

## 5. Ferramenta de Limpeza

Comando oficial de limpeza:

- `lua tools/selfhost_cleanup.lua`

Limpeza ampliada para sobras ignoradas no diretorio historico:

- `lua tools/selfhost_cleanup.lua --strict-selfhost-scratch`

## 6. Leitura complementar

- `selfhost-abi.md`
- `current-core.md`
- `../roadmap/selfhost-100.md`
- `../roadmap/selfhost-pos100.md`
