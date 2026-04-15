# Roadmap de Ascensao: Linha Self-Hosted

> Estado: historico concluido e superado pelo corte oficial.
> Leitura corrente: `selfhost-100.md`, `selfhost-pos100.md` e `../specification/current-core.md`.

Ascension foi a trilha que tirou o compilador self-hosted do campo experimental e preparou a troca do produto oficial. Este arquivo nao mede mais maturidade atual; ele preserva a fase de transicao que antecedeu a oficializacao e o corte 100% self-hosted.

## O que Ascension entregou

- consolidacao de `src/compiler/syntax.zt` como compilador self-hosted canonico;
- fechamento do bootstrap deterministico e da paridade minima do core;
- reducao estrutural de `native lua` no core e na stdlib canonica;
- base para o corte posterior do front door oficial.

## O que veio depois

- `selfhost-oficializacao.md`: promoveu a linha self-hosted a face oficial do produto;
- `selfhost-100.md`: eliminou fallback legado do caminho oficial;
- `selfhost-pos100.md`: limpou artefatos, endureceu builtins e fechou a leitura institucional.

## Como ler este documento agora

Use Ascension como registro de transicao, nao como status page.

Se a pergunta for "qual e o estado atual da linguagem e do compilador?", leia primeiro:

- `../specification/current-core.md`
- `selfhost-100.md`
- `selfhost-pos100.md`
- `../compiler/ascension-zenith.md`

## Resultado historico preservado

Ascension deixou de ser promessa. Ela virou a fase que permitiu:

- oficializacao do compilador em Zenith;
- gate de release self-hosted;
- auditoria de legado no caminho oficial;
- reposicionamento da trilha Lua como runtime e implementacao de referencia.

*Atualizado em: 2026-04-15*
