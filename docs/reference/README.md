# Reference Docs

> Referencias consultaveis da linguagem e do ecossistema.
> Audience: package-author, contributor, advanced-user
> Surface: reference

## Objetivo

Guardar regras curtas, estaveis e faceis de consultar.

Use esta pasta para:

- referencia de sintaxe;
- referencia de tipos;
- diagnosticos;
- modelo de projeto;
- APIs estaveis;
- knowledge base.

Se a pergunta for normativa para implementacao do compilador, consulte tambem `language/spec/`.

## Secoes

- `language/`: referencias publicas da linguagem.
- `stdlib/`: referencias publicas da stdlib.
- `cli/`: referencia de CLI, diagnosticos e tooling.
- `api/`: destino reservado para documentacao gerada por ZDoc.
- `zenith-kb/`: base curta de conhecimento por area.

## Entrada rapida

| Preciso de | Leia |
| --- | --- |
| sintaxe da linguagem | `docs/reference/language/syntax.md` |
| tipos e genericos | `docs/reference/language/types.md` |
| `public`, `public var`, imports | `docs/reference/language/modules-and-visibility.md` |
| funcoes e controle de fluxo | `docs/reference/language/functions-and-control-flow.md` |
| `optional` e `result` | `docs/reference/language/errors-and-results.md` |
| comandos `zt` | `docs/reference/cli/zt.md` |
| comandos `zpm` | `docs/reference/cli/zpm.md` |
| modulos da stdlib | `docs/reference/stdlib/modules.md` |
| I/O e JSON | `docs/reference/stdlib/io-json.md` |
| texto, bytes e formatacao | `docs/reference/stdlib/text-bytes-format.md` |
| filesystem, OS e tempo | `docs/reference/stdlib/filesystem-os-time.md` |
| collections | `docs/reference/stdlib/collections.md` |
| math, regex, random e validate | `docs/reference/stdlib/math-random-validate.md` |
| concurrent, lazy, test e net | `docs/reference/stdlib/concurrency-lazy-test-net.md` |
