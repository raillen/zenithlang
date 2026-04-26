# Zenith Language Roadmap 6.0 (Ergonomics Without Magic)

> Roadmap do ciclo R6 da linguagem Zenith.
> Status: proposto.
> Criado: 2026-04-25.

## Objetivo

R6 melhora ergonomia, legibilidade e acessibilidade sem enfraquecer a filosofia
da linguagem.

O foco deste ciclo e reduzir atrito real de uso com:

- diagnosticos mais uteis;
- formatter mais reading-first;
- pequenos helpers explicitos;
- melhor documentacao do caminho Zenith para recursos comuns em outras linguagens.

R6 nao adiciona inferencia local ampla, overloads, macros, heranca classica ou
atalhos simbolicos densos.

## Principios do ciclo

- Explicito antes de implicito.
- Leitura antes de concisao.
- Sintaxe nova so quando o ganho for claro e o custo cognitivo for baixo.
- Builtin so quando precisa do compilador.
- Funcoes auxiliares comuns ficam em `std.*`.
- Erro bom vale como feature de linguagem.

## Escopo recomendado

### R6.M0 - Baseline de filosofia e nao-objetivos

Congelar o que nao sera implementado neste ciclo.

- Manter declaracoes locais com tipo explicito.
- Manter `enum` com payload como resposta oficial para union segura.
- Manter `trait` como contrato abstrato.
- Manter `dyn<Trait>` como dispatch dinamico explicito.
- Manter `apply` como extensao de comportamento sem partial class.
- Nao adicionar `abstract`, `virtual`, `partial`, `union`, `?.`, `??`,
  ternario, pipe operator, implicit return, overload ou macros.

Entrega esperada:

- documento curto explicando equivalentes Zenith para conceitos vindos de
  C#, Java, TypeScript, Rust e Go;
- diagnosticos ou mensagens de parser apontando para esses equivalentes quando
  houver erro comum.

### R6.M1 - Diagnosticos de sintaxe nao-canonica

Adicionar sugestoes diretas para erros comuns de quem vem de outras linguagens.

Casos recomendados:

- `string` -> use `text`;
- `let` -> use `const` ou `var`;
- `&&` -> use `and`;
- `||` -> use `or`;
- `!value` -> use `not value`;
- `null` -> use `optional<T>` e `none`;
- `throw` -> use `result<T,E>`, `error(...)` ou `panic(...)` conforme o caso;
- `abstract` -> use `trait`;
- `virtual` -> use `dyn<Trait>` quando precisar de dispatch dinamico;
- `union` -> use `enum` com payload;
- `partial` -> use `apply` e organize por namespace/arquivo.

Entrega esperada:

- mensagens action-first;
- fixtures invalidas com fragmentos esperados;
- cobertura no LSP quando o erro passar pelo mesmo pipeline.

### R6.M2 - Builtins pequenos de intencao

Adicionar apenas builtins que representam intencao central e precisam de
tratamento do compilador ou lowering dedicado.

Builtins recomendados:

```zt
todo("mensagem")
unreachable("mensagem")
check(condition, "mensagem opcional")
```

Regras:

- `todo` e `unreachable` baixam para falha fatal clara;
- `check(condition)` continua valido;
- `check(condition, message)` melhora diagnostico/teste sem criar assert alias;
- `assert` nao entra como alias neste ciclo.

### R6.M3 - `std.test` mais expressivo

Melhorar testes sem adicionar nova sintaxe.

Helpers recomendados:

- `test.is_true(value)`;
- `test.is_false(value)`;
- `test.equal_int(actual, expected)`;
- `test.equal_text(actual, expected)`;
- `test.not_equal_int(actual, expected)`;
- `test.not_equal_text(actual, expected)`.

Meta:

- falhas devem mostrar valor esperado, valor recebido e proxima acao;
- exemplos devem ser curtos e legiveis.

### R6.M4 - Helpers explicitos para `optional` e `result`

Reduzir `match` repetitivo sem esconder fluxo de erro.

Helpers recomendados:

```zt
maybe_value.or(default_value)
maybe_value.or_error("mensagem")
maybe_value.is_some()
maybe_value.is_none()
result_value.is_success()
result_value.is_error()
result_value.or_panic("mensagem")
```

Fatia implementada agora:

- `optional<T>.or(default_value)`;
- `optional<T>.is_some()`;
- `optional<T>.is_none()`;
- `result<T,E>.is_success()`;
- `result<T,E>.is_error()`.

Fatia especificada para depois:

- `optional<T>.or_error(message)`;
- `result<T,E>.or_panic(message)`.

Regras:

- `match` continua preferido quando o valor precisa ser extraido com logica;
- nao adicionar `?.` ou `??`;
- nao converter `optional` para `result` implicitamente;
- erros recuperaveis continuam usando `result<T,E>`.

### R6.M5 - Helpers pequenos em `std.validate`, `std.text` e colecoes

Adicionar funcoes auxiliares claras, nao sintaxe.

`std.validate`:

- `positive(value)`;
- `non_negative(value)`;
- `not_empty_text(value)`;
- `min_len(value, min)`;
- `max_len(value, max)`;
- `between(value, min, max)`.

`std.text`:

- `is_empty(value)`;
- `is_blank(value)`;
- `contains(value, needle)`;
- `starts_with(value, prefix)`;
- `ends_with(value, suffix)`.

Colecoes:

- `list.is_empty(items)`;
- `map.is_empty(values)`;
- `map.has_key(values, key)`.

Meta:

- nomes claros;
- comportamento previsivel;
- sem abreviacoes cripiticas;
- testes behavior para cada modulo alterado.

Status em 2026-04-26:

- implementado em `std.validate`, `std.text`, `std.list` e `std.map`;
- coberto por `tests/behavior/std_small_helpers/`;
- `std.list` e `std.map` sao modulos pequenos para helpers compiler-known de colecoes built-in.

### R6.M6 - Warnings de legibilidade e acessibilidade

Expandir warnings que reduzem erro visual e confusao.

Warnings recomendados:

- nomes muito parecidos no mesmo escopo;
- identificadores soltos como `l`, `I`, `O`, `0` quando aceitos pelo lexer;
- shadowing perigoso;
- bloco muito profundo;
- funcao muito longa;
- `match` com `case default` quando enum conhecido poderia ser exaustivo.

Regras:

- warning nao deve bloquear build fora de modo estrito;
- mensagem deve explicar risco e proxima acao;
- evitar falso positivo agressivo.

Status parcial em 2026-04-26:

- `name.confusing` cobre identificadores visualmente confusos.
- `name.similar` cobre nomes locais muito parecidos no mesmo escopo.
- `style.block_too_deep` cobre blocos com profundidade maior que 6.
- `style.function_too_long` cobre funcoes com mais de 80 instrucoes.
- `control_flow.enum_default_case` cobre `case default` em `match` de enum conhecido.
- modo normal reporta warnings sem bloquear;
- `[diagnostics] profile = "strict"` promove warnings para erro;
- shadowing continua erro duro no MVP (`name.shadowing`), sem rebaixar para warning.

### R6.M7 - Formatter reading-first

Melhorar layout como feature de acessibilidade.

Regras recomendadas:

- quebrar assinatura longa em parametros um por linha;
- manter `where` longo em linha separada quando melhorar leitura;
- preservar comentario junto do item que ele explica;
- manter `match` vertical e estavel;
- evitar oscilacao de formatter.

Gate obrigatorio:

- `fmt(fmt(x)) == fmt(x)` para fixtures novas.

Status atual: implementado com fixture `case_reading_first` no golden test e no gate de idempotencia.

### R6.M8 - `zt explain`

Adicionar explicacao offline para diagnosticos.

Exemplo:

```powershell
./zt.exe explain type.mismatch
```

Saida esperada:

- significado curto;
- exemplo invalido;
- exemplo corrigido;
- proxima acao;
- link local de doc quando existir.

Status atual: implementado com `zt explain <codigo>`, catalogo inicial no driver e teste `tests/driver/test_explain_cli.py`.

### R6.M9 - Polimento de `enum`, callables e `dyn`

Nao adicionar novos nomes de sintaxe.

Melhorar o que ja existe:

- erro de `match` enum nao exaustivo deve listar cases ausentes;
- erro de callable deve mostrar assinatura esperada e recebida;
- erro de `dyn<Trait>` deve explicar quando usar generics/trait constraints;
- docs devem explicar:
  - union segura = `enum` com payload;
  - metodo abstrato = `trait`;
  - virtual = `dyn<Trait>`;
  - partial class = `apply` + namespaces/arquivos.

Status atual: implementado. `control_flow.non_exhaustive_match` lista variants ausentes, `callable.signature_mismatch` mostra assinatura esperada/recebida, `callable.invalid_func_ref` orienta wrapper nao generico e diagnosticos `dyn.*` sugerem generics com `where` quando dispatch dinamico nao e necessario.

## Escopo explicitamente fora de R6

Nao entram neste ciclo:

- local type inference ampla;
- `union` como nova keyword;
- `abstract` como keyword;
- `virtual` como keyword;
- `partial` como keyword;
- partial application;
- callable em struct field ou container;
- closures com captura mutavel;
- `?.` safe navigation;
- `??` coalescing;
- ternario;
- pipe operator;
- overload;
- macros;
- implicit return.

## Gates de qualidade

Antes de fechar R6:

- `python build.py`;
- `./zt.exe check zenith.ztproj --all --ci`;
- `python run_suite.py smoke --no-perf`;
- suites de behavior novas para cada helper;
- fixtures invalidas para diagnosticos novos;
- formatter golden + idempotence para regras novas;
- docs atualizadas em `language/spec`, `language/decisions` ou `docs/reference`
  quando a superficie publica mudar.

## Criterio de fechamento

R6 fecha quando:

- nenhuma sintaxe pesada foi adicionada sem RFC propria;
- os diagnosticos novos ajudam a migrar para o estilo Zenith;
- os helpers novos reduzem boilerplate real sem esconder semantica;
- os exemplos continuam curtos, explicitos e acessiveis;
- os gates principais estao verdes.

Status atual: fechado localmente em
`docs/internal/reports/release/R6-clarity-and-accessibility-closure-report.md`.

Evidencia principal:

- `python build.py`;
- `./zt.exe check zenith.ztproj --all --ci`;
- `python run_suite.py smoke --no-perf` (`13/13`);
- `python run_suite.py pr_gate --no-perf` (`198/198`).
