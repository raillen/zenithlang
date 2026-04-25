# Changelog

Todos os marcos do desenvolvimento da linguagem Zenith.

## [0.3.0-alpha.3-rc.1] - 2026-04-24

Release candidate do ciclo R3.

Status: **nao publicado como release final**.

Motivo: artefato e install limpo passaram, mas os gates de release ainda estao vermelhos.

Notas e evidencias:

- `docs/reports/release/0.3.0-alpha.3-rc.1-notes.md`
- `docs/reports/release/0.3.0-alpha.3-rc.1-release-report.md`
- `docs/reports/compatibility/0.3.0-alpha.3-rc.1-compatibility.md`
- `docs/reports/release/R3.M9-known-limits-and-risk.md`
- `docs/reports/R3.M9-borealis-final-alignment.md`

### Adicionado

- `public var` de namespace com leitura externa qualificada e escrita restrita ao namespace dono.
- primeiro corte de concorrencia com contrato de copia em fronteira.
- FFI 1.0 Phase 1 com ABI, ownership e shielding documentados.
- subset minimo de `dyn Trait`.
- callables tipados e delegates.
- closures v1 com captura imutavel.
- lambdas de expressao e HOFs inteiras em `std.collections`.
- `lazy<int>` explicito e one-shot em `std.lazy`.

### Validacao

- `python build.py` verde.
- pacote Windows AMD64 gerado.
- checksum SHA-256 gerado.
- install limpo validado a partir do zip.
- `tests/perf/gate_pr.ps1` vermelho.
- `tests/perf/gate_nightly.ps1` vermelho.

### Bloqueadores Para Release Final

- `behavior/borealis_backend_fallback_stub`.
- `hardening/differential_validate_between`.
- drift de baseline em `perf/quick` e `perf/nightly`.

## [0.3.0-alpha.2] - 2026-04-23

Ciclo de fechamento das 10 pendências corretivas residuais (`PLI-01` a `PLI-10`) documentadas em `reports/pending-language-issues-current.md`. Build verde (`python build.py`), `smoke` verde (`10/10`), `pr_gate` backend `6/6` e `pr_gate` tooling `4/4` (golden + idempotence).

Notas e evidências:

- `docs/reports/release/0.3.0-alpha.2-notes.md`
- `docs/reports/release/0.3.0-alpha.2-release-report.md`
- `docs/reports/compatibility/0.3.0-alpha.2-compatibility.md`

### Adicionado

- **gate de idempotência do formatter** (`PLI-04`):
  - `tests/formatter/run_formatter_idempotence.py` verifica `fmt(fmt(x)) == fmt(x)` em todos os casos canônicos (`tests/formatter/cases/`).
  - integrado ao `run_suite.py` dentro de `pr_gate` (tooling).
  - divisão original `IDEMPOTENT` vs `XFAIL` foi eliminada: todos os 9 casos convergem após as correções.
- **detecção de colisão de símbolos no emitter C** (`PLI-02`, `PLI-09`):
  - guarda explícita em `compiler/targets/c/emitter.c` que aborta com diagnóstico quando dois símbolos distintos de Zenith geram o mesmo nome mangled em C.
  - novo teste unitário `tests/targets/c/test_emitter_symbol_collision.c` cobrindo: função vs. struct homônimos, colisão dot-vs-underscore e baseline sem colisão. Descoberto automaticamente pelo runner.
- **limite estrutural no lexer** (`PLI-01`, `PLI-08`):
  - `ZT_LEX_MAX_STRING_LEN = 1 MiB` em `compiler/frontend/lexer/lexer.c`.
  - `zt_lexer_read_string`, `zt_lexer_resume_string` e `zt_lexer_read_triple_quoted` emitem `lexer.token_too_long`/`lexer.unterminated_string` via `zt_lexer_emit_diag`.

### Corrigido

- **`PLI-10` — formatter não-idempotente em 6 sub-casos distintos**:
  - string literal perdia aspas: formatter agora sempre emite aspas ao serializar `ZT_AST_STR`.
  - parâmetros de tipo eram emitidos como `[T]`: trocado para `<T>` conforme parser e `surface-syntax.md`.
  - `match`/`case` ganhava `end` espúrio após cada corpo de case: corpo de case agora não fecha prematuramente.
  - enums emitiam vírgula entre variantes, quebrando parse pós-fmt: vírgula removida (uma variante por linha).
  - cláusula `where` emitia `where <expr> end` em vez de apenas a expressão: alinhado ao parser.
  - roubo de comentários por nodes AST adjacentes causava oscilação `fmt^n`: parser passa a usar padrão stash/reclaim/apply escopado (`compiler/frontend/parser/parser.c`), whitelist de nodes que aceitam leading comment e restauração de trailing comments nos loops de imports/declarações/bloco.
- **`PLI-02` / `PLI-09` — buffers truncados no emitter**:
  - buffers internos de partes de símbolo mangled ampliados de `64` para `256` bytes (`C_EMIT_SYMBOL_PART_MAX`), evitando truncamento silencioso com paths de namespace profundos.
- **`PLI-07` — use-after-free no driver**:
  - removido o `free(source_text)` prematuro em `compiler/driver/project.c`; o release passa pelo dispose da lista de fontes, mantendo ponteiros de comment tokens válidos durante `fmt --check` e emissões subsequentes.
- **`PLI-03`, `PLI-05`, `PLI-06`**: resolvidos em lote (detalhes em `reports/pending-language-issues-current.md`).

### Qualidade e Validação

- `python build.py` verde.
- `python run_suite.py smoke` -> `10/10`.
- `python run_suite.py pr_gate` backend -> `6/6` (inclui `test_emitter_symbol_collision`).
- `python run_suite.py pr_gate` tooling -> `4/4` (formatter golden + formatter idempotence).
- todas as fixtures em `tests/formatter/cases/*/expected/` foram promovidas à saída canônica pós-correções.
- relatório de pendências atualizado: 0 pendências `P0`/`P1`/`P2`/`P3` abertas, 10 corrigidas.

## [0.3.0-alpha.1] - 2026-04-21

### Added
- alpha release package for Windows AMD64:
  - published as GitHub Release asset (outside source repository)
  - file names: `zenith-0.3.0-alpha.1-windows-amd64.zip` and `zenith-0.3.0-alpha.1-windows-amd64.checksums.txt`
- alpha release notes and gate evidence:
  - `docs/reports/release/0.3.0-alpha.1-notes.md`
  - `docs/reports/release/R2.M12-alpha-release-report.md`
- compatibility record for alpha cut:
  - `docs/reports/compatibility/R2.M12-alpha-compatibility.md`

### Quality And Validation
- PR gate green:
  - `tests/perf/gate_pr.ps1` (`117/117`)
- fuzz/corpus without new crashes:
  - `python tests/fuzz/replay.py --verbose` -> `seeds=0`, `failures=0`
  - `python tests/fuzz/fuzz_lexer.py --iters 200 --seed 20260421 --verbose` -> `crashes=0`, `timeouts=0`
  - `python tests/fuzz/fuzz_parser.py --iters 200 --seed 20260421 --verbose` -> `crashes=0`, `timeouts=0`
- clean install and hello world from packaged artifact:
  - `zt check`/`zt build`/`zt run` all with exit `0`

### Known Limits
- `stdlib/platform/` remains internal placeholder and not a public API.
- pre-`1.0.0` compatibility policy remains "breaking changes allowed between pre-releases".

## [0.3.0-rc.1] - 2026-04-21

### Adicionado
- Feature pack A completo (R2.M9):
  - `panic(...)` como statement de fonte
  - `optional<T>?` com propagacao de `none`
  - match de optional com `case value` e exhaustividade de enum
- `fmt "..."` completo ponta a ponta (parser, semantic, lowering, emitter, formatter, testes)
- aliases canonicos de inteiros sem sinal: `u8`, `u16`, `u32`, `u64`

### Corrigido
- coerencia de `check(...)` como intrinsic e validacao de tipo
- construcao qualificada de `core.Error(...)` no fluxo completo
- `Comparable` no prelude/checker e operadores relacionais alinhados por trait
- classificacao de suite para `fmt_interpolation_type_error` no gate `pr_gate/nightly/stress`

### Qualidade E Performance
- suites obrigatorias de RC verdes:
  - `python run_suite.py smoke` (`9/9`)
  - `python run_suite.py pr_gate` (`112/112`)
  - `python run_suite.py nightly` (`114/114`)
  - `python run_suite.py stress` (`21/21`)
- performance nightly em `pass` com `23` benchmarks (`reports/perf/summary-nightly.json`)

## [0.2.5] - 2026-04-09

### Adicionado (Stdlib Foundation)
- 🖥️ **std.os**: Interface com o sistema operacional, hardware e variáveis de ambiente.
- 📂 **std.fs**: Manipulação de arquivos e pastas (nível atômico e streams).
- 🧭 **std.fs.path**: Submódulo para manipulação inteligente de caminhos.
- 📦 **std.json**: Serialização e desserialização de dados estruturados com suporte a indentação.

### Corrigido (Estabilização do Core)
- 🐛 **Shadowing no Codegen**: Corrigida a declaração dupla de locais que quebrava a reatividade.
- 🏗️ **Aninhamento no Parser**: Corrigido o `parse_block` para suportar estruturas complexas aninhadas (match/while/if).
- 🔤 **Lexer Escapes**: Adicionado suporte a escapes de aspas e chaves (`\{`, `\}`) em strings literais.
- 🛠️ **Operadores no Codegen**: Adicionado suporte para operador de tamanho `#`, listas `[]` e mapas `{}`.

## [0.2.0] - 2026-04-08

### Adicionado (Finalização do Core)
- 🏗️ **Lowering & IR**: Fase de transformação intermediária para desaçucaramento e otimização.
- ⚡ **Constant Folding**: Otimização automática de expressões constantes em tempo de compilação.
- 🧩 **Destruturação Completa**: Suporte para desmontar listas e structs em `match` e declarações `var`.
- 🧬 **Genéricos com Restrições**: Introdução de `where T is Trait` para segurança em tipos genéricos.
- 🛡️ **Traits Avançadas**: Validação de satisfatibilidade e Implementações Padrão (Default Impls).
- 🚦 **Operador `where`**: Contratos de dados em runtime validados automaticamente em structs.
- 📦 **Sistema de Projeto**: Lançamento do formato `.ztproj` para orquestração de builds.
- 🎹 **Prelude Automática**: Carregamento de `std.core` sem necessidade de import explícito.
- 🔪 **Slicing e Spread**: Manipulação poderosa de coleções com operadores `..` e `[start..end]`.
- 🏷️ **Keywords Type e Union**: Distinção clara entre apelidos semânticos e tipos soma.

### Alterado
- 🍭 **Açúcar de Sintaxe**: Substituído `.campo` por `@campo` para acesso ao `self`.
- 🔗 **Chaining Vertical**: O ponto `.` agora é usado exclusivamente para encadeamento de funções (UFCS).
- 💎 **Tipagem Obrigatória**: Declarações de variáveis agora exigem tipo explícito (`var x: int`).
- ➡️ **Sintaxe de Retorno**: Padronização do uso de `->` para tipos de retorno em funções.

### Corrigido
- 🐛 Correção de bugs críticos no Binder relacionados a dependências circulares.
- 🐛 Estabilização do Lexer para lidar com interpolações complexas e caracteres especiais.

## [0.1.0] - 2026-04-07
- Versão inicial com suporte básico a variáveis, funções e structs.
- Transpilação básica para Lua.
