# Zenith Next Language

Esta area guarda decisoes, specs consolidados e documentos normativos da linguagem Zenith Next.

Regras desta area:

- cada decisao de sintaxe ou tooling vira um documento proprio em `decisions/`
- specs canonicos consolidados entram em `spec/`
- o objetivo e priorizar legibilidade, previsibilidade e baixo atrito neural
- a surface syntax deve servir a semantica da linguagem, nao ao backend

Documentos canonicos consolidados:

- `spec/README.md`: indice dos specs canonicos.
- `spec/surface-syntax.md`: sintaxe e semantica visivel ao usuario.
- `spec/project-model.md`: `zenith.ztproj`, app/lib, layout, ZDoc e ZPM.
- `spec/compiler-model.md`: pipeline, ZIR, backend C, runtime e artefatos.
- `spec/tooling-model.md`: CLI `zt`, `zpm`, diagnostics, formatter, testes e docs.

Documentos de status:

- `surface-implementation-status.md`: status da implementacao atual contra os specs.
- `MVP_OUT_OF_SCOPE.md`: itens explicitamente fora do corte inicial.

Historico e racional:

- `decisions/`: decisoes aceitas, rascunhos e contexto de tradeoffs.

Compatibilidade:

- `surface-spec.md` foi substituido pelos specs em `spec/` e permanece apenas como ponteiro.
