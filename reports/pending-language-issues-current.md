# Relatório Atual de Pendências Reais — Zenith Lang v2

**Data:** 2026-04-23  
**Versão analisada:** 0.3.0-alpha.1  
**Escopo:** somente itens ainda abertos após revalidar os relatórios legados de 2026-04-22 e revalidação adicional em 2026-04-23  
**Status deste arquivo:** fonte atual para pendências corretivas residuais

## Histórico recente

- 2026-04-23 (lote 1): `PLI-01`, `PLI-03`, `PLI-05`, `PLI-06` e `PLI-08` marcados como `Corrigido`. Build verde (`python build.py`), suite `smoke` verde (10/10). Detalhes em cada item.
- 2026-04-23 (lote 2): `PLI-07` marcado como `Corrigido`. Build verde, suite `smoke` verde (10/10). Detalhes no item.
- 2026-04-23 (lote 3): `PLI-04` marcado como `Corrigido` (gate de idempotência do formatter implementado em `tests/formatter/run_formatter_idempotence.py` e integrado ao `run_suite.py`). Durante a implementação, o gate descobriu 5 casos reais de não-idempotência no formatter: aberto como `PLI-10`.
- 2026-04-23 (lote 4): `PLI-02` e `PLI-09` marcados como `Corrigido`. Build verde, smoke verde, pr_gate backend pass=6/6, novo teste unit `test_emitter_symbol_collision.exe` adicionado e descoberto automaticamente pelo runner.
- 2026-04-23 (lote 5): `PLI-10` marcado como `Corrigido`. Build verde, pr_gate tooling pass=4/4 (formatter golden + idempotence), todos os 9 casos de idempotência convergem. Seis bugs distintos do formatter/parser foram endereçados.

---

## 1. Como ler este relatório

Este arquivo mantém apenas o que ainda precisa de ação real hoje.

Ele não repete:

- bugs já corrigidos;
- decisões arquiteturais já aceitas;
- hipóteses fracas sem evidência suficiente;
- hardening já incorporado ao runner oficial sem bug confirmado.

Resumo rápido:

- `P0`: 0
- `P1`: 0
- `P2`: 0 (abertos) + 4 (corrigidos)
- `P3`: 0 (abertos) + 6 (corrigidos)

**Todas as 10 pendências correctivas originais foram resolvidas.** Este arquivo agora serve apenas como histórico até o próximo ciclo de auditoria.

---

## 2. Pendências ainda abertas

### PLI-01 — String sem fechamento ainda pode crescer sem limite explícito no lexer
- Prioridade: `P2`
- Área: `Frontend / Hardening`
- Status: `Corrigido` (2026-04-23)
- Resolução:
1. Adicionada constante `ZT_LEX_MAX_STRING_LEN = 1 MiB` em `compiler/frontend/lexer/lexer.c`.
2. `zt_lexer_read_string`, `zt_lexer_resume_string` e `zt_lexer_read_triple_quoted` agora emitem `lexer.token_too_long` via `zt_lexer_emit_diag` ao exceder o limite e retornam `ZT_TOKEN_LEX_ERROR`.
3. Caminho casado com `PLI-08` (string mal-fechada) também reporta `lexer.unterminated_string`.
- Evidência histórica:
1. `zt_lexer_read_string()` continua lendo até newline/EOF.
2. `zt_lexer_read_triple_quoted()` continua lendo até delimitador/EOF.
3. Não há limite estrutural explícito de tamanho para esse caminho.
- Arquivo principal: `compiler/frontend/lexer/lexer.c`
- Risco real:
1. consumo excessivo de memória em input malicioso ou corrompido;
2. degradação de tooling, fuzz e host integrations;
3. abertura para DoS local no compilador.
- O que corrigir:
1. definir um limite estrutural claro para literal de string em análise léxica;
2. emitir diagnóstico estável quando o limite for excedido;
3. manter caminhos normais sem custo perceptível.
- Plano de implementação:
1. adicionar constante única de limite léxico para strings;
2. aplicar o mesmo guard em string simples e triple-quoted;
3. emitir erro dedicado, sem continuar acumulando buffer indefinidamente;
4. alinhar a mensagem com o catálogo de diagnósticos do compilador.
- Testes mínimos:
1. string simples sem fechamento acima do limite;
2. triple-quoted sem fechamento acima do limite;
3. strings grandes, mas válidas, logo abaixo do limite.

### PLI-02 — Sanitização de símbolos do emitter C ainda pode colidir nomes distintos
- Prioridade: `P2`
- Área: `Backend C / Corretude`
- Status: `Corrigido` (2026-04-23)
- Resolução:
1. Novo guard `c_check_symbol_collisions` em `compiler/targets/c/emitter.c` é chamado no início de `c_emitter_emit_module`.
2. O guard percorre todas as declarações públicas (functions, structs, enums) do módulo, mangla cada uma com as mesmas rotinas usadas pelo emitter (`c_build_function_symbol`, `c_build_struct_symbol`, `c_build_enum_symbol`), e falha com diagnóstico `C_EMIT_INVALID_INPUT` quando duas `(kind, source)` distintas chegam ao mesmo símbolo C.
3. A mensagem de erro inclui os dois nomes-fonte originais e o símbolo C colidido, direcionando o usuário para renomear uma das declarações.
4. Novo teste unitário `tests/targets/c/test_emitter_symbol_collision.c` cobre três cenários: function+struct com mesmo nome no mesmo módulo, dois structs `a.b` vs `a_b` (colapsão de `.`/`_`), e caso normal sem falso-positivo.
5. A sanitização interna foi mantida para preservar goldens existentes — a decisão foi tornar a colisão um erro explícito em vez de reescrever o encoder, o que teria impacto massivo em golden tests.
- Evidência histórica:
1. a sanitização ainda converte caracteres fora de `[A-Za-z0-9_]` para `_`;
2. isso mantém o risco de dois nomes fonte diferentes virarem o mesmo símbolo C.
- Arquivo principal: `compiler/targets/c/emitter.c`
- Risco real:
1. colisão silenciosa entre módulos/funções/declarações;
2. geração incorreta ou ambígua em projetos maiores;
3. regressão difícil de rastrear, porque o código fonte Zenith continua aparentemente válido.
- O que corrigir:
1. trocar sanitização “achatadora” por encoding reversível de símbolo;
2. ou detectar colisão e falhar com diagnóstico explícito.
- Plano de implementação:
1. criar encoder estável de nomes para símbolos C;
2. garantir que `.` e `_` não caiam no mesmo resultado;
3. adicionar tabela temporária de símbolos emitidos para detectar colisões remanescentes;
4. cobrir nomes com namespace, underscore, hífen sanitizado e caracteres especiais aceitos na superfície correspondente.
- Testes mínimos:
1. caso de colisão potencial entre `app.main` e `app_main`;
2. caso com múltiplos módulos de nomes próximos;
3. golden test do emitter validando estabilidade do símbolo gerado.

### PLI-03 — Mensagens de contrato ainda usam buffer fixo de 512 bytes
- Prioridade: `P3`
- Área: `Runtime / Diagnostics`
- Status: `Corrigido` (2026-04-23)
- Resolução:
1. Novo helper estático `zt_contract_failed_with_suffix` em `runtime/c/zenith_rt.c` aloca o buffer final via `malloc(base_len + suffix_len + 1)`.
2. `zt_contract_failed_i64`, `zt_contract_failed_float` e `zt_contract_failed_bool` foram simplificados para montar apenas o sufixo curto (`(value: ...)`) e delegar a concatenação.
3. Fallback para buffer de stack só em caso de falha de alocação; buffer dinâmico é intencionalmente vazado no panic, que é terminal.
- Evidência histórica:
1. helpers de falha de contrato ainda montam a mensagem em `char full_message[512]`.
- Arquivo principal: `runtime/c/zenith_rt.c`
- Risco real:
1. truncamento de mensagem em cenários longos;
2. perda de contexto de depuração;
3. diagnósticos piores justamente em casos complexos.
- O que corrigir:
1. sair de buffer fixo para montagem com tamanho calculado ou builder interno;
2. preservar mensagem completa sem UB.
- Plano de implementação:
1. medir o tamanho necessário com `snprintf`;
2. alocar buffer do tamanho exato, ou usar helper interno de string builder do runtime;
3. manter fallback simples e seguro para OOM;
4. revisar helpers irmãos para manter padrão único.
- Testes mínimos:
1. mensagem curta sem regressão;
2. mensagem longa acima de 512 bytes;
3. valores `i64`, `float` e `bool`.

### PLI-04 — Formatter ainda não tem gate formal de idempotência
- Prioridade: `P3`
- Área: `Formatter / Testes`
- Status: `Corrigido` (2026-04-23)
- Resolução:
1. Novo runner `tests/formatter/run_formatter_idempotence.py` copia cada projeto de `tests/formatter/cases/*/input` para `.ztc-tmp/tests/formatter-idempotence/`, roda `zt fmt` duas vezes e compara as duas saídas.
2. Casos são classificados em duas listas: `IDEMPOTENT_CASES` (obrigatórios, quebrar é hard fail) e `XFAIL_CASES` (não convergem ainda, rastreados em `PLI-10`).
3. Um caso `XFAIL` que passe a convergir fica rotulado `[XOK ]` e falha o gate exigindo promoção para `IDEMPOTENT_CASES` — isso evita que o conjunto de proteção estagne.
4. `run_suite.py:run_formatter_section` agora roda golden + idempotence em sequência; a suite `tooling` subiu de 3 para 4 testes e continua verde no `pr_gate`.
5. Documentação viva: mudar a superfície do formatter exige adicionar novos fixtures em `IDEMPOTENT_CASES`.
- Evidência histórica:
1. existem goldens e cobertura boa de formatter;
2. mas não há ainda um gate explícito de propriedade `fmt(fmt(x)) == fmt(x)`.
- Arquivos principais:
1. `language/spec/formatter-model.md`
2. `tests/formatter/run_formatter_golden.py`
- Risco real:
1. regressão sutil de formatação passar pelos goldens;
2. comportamento “oscilante” entre duas formas válidas.
- O que corrigir:
1. transformar idempotência em propriedade verificada automaticamente.
- Plano de implementação:
1. escolher subconjunto inicial de casos canônicos;
2. rodar formatter duas vezes nos mesmos fixtures;
3. falhar se a segunda saída diferir da primeira;
4. depois ampliar o conjunto com novos casos conforme a superfície crescer.
- Testes mínimos:
1. imports;
2. structs;
3. match;
4. comentários;
5. triple-quoted text;
6. manifest.

### PLI-05 — O lexer ainda não aplica um limite uniforme para tokens muito longos
- Prioridade: `P3`
- Área: `Frontend / Hardening`
- Status: `Corrigido` (2026-04-23)
- Resolução:
1. Constante `ZT_LEX_MAX_TOKEN_LEN = 1024` definida em `compiler/frontend/lexer/lexer.c`.
2. `zt_lexer_read_identifier` e `zt_lexer_read_number` agora verificam o comprimento acumulado e emitem `lexer.token_too_long` quando excede.
3. Diagnóstico passa pelo canal `zt_lexer_set_diagnostics` — o lexer principal do parser agora registra esse canal (`compiler/frontend/parser/parser.c:zt_parse`).
- Evidência histórica:
1. o diagnóstico `lexer.token_too_long` existe no catálogo;
2. mas não há aplicação uniforme desse limite no lexer para identificadores, números e outros tokens longos;
3. hoje esse diagnóstico aparece em usos específicos do parser, não como guard geral de tokenização.
- Arquivos principais:
1. `compiler/frontend/lexer/lexer.c`
2. `compiler/semantic/diagnostics/diagnostics.c`
- Risco real:
1. consumo excessivo de memória com input hostil;
2. comportamento desigual entre classes de token;
3. drift entre o catálogo de diagnósticos e o que o lexer realmente garante.
- O que corrigir:
1. definir uma política única de tamanho máximo por token;
2. aplicar o guard no lexer, não só em pontos isolados do parser.
- Plano de implementação:
1. introduzir constante única para limite léxico;
2. verificar comprimento acumulado durante leitura de identificadores, números e strings;
3. emitir `lexer.token_too_long` de modo consistente;
4. alinhar a mensagem e os testes com esse contrato.
- Testes mínimos:
1. identificador acima do limite;
2. literal numérico acima do limite;
3. string acima do limite;
4. token logo abaixo do limite, para garantir ausência de regressão.

### PLI-06 — Chaves duplicadas no manifesto ainda podem sobrescrever silenciosamente
- Prioridade: `P3`
- Área: `Driver / Manifest`
- Status: `Corrigido` (2026-04-23)
- Resolução:
1. `zt_project_parse_text` em `compiler/project/ztproj.c` agora mantém um array local de `(section, key)` já assinalados (hard cap de 128 entries).
2. Ao encontrar duplicata antes de chamar `zt_project_assign_value`, emite `project.invalid_assignment` com mensagem `"duplicate key 'section.key' in manifest"`.
3. Seções de dependência (`[dependencies]`, `[dev_dependencies]`) são deliberadamente puladas porque sua semântica de múltiplas entradas é tratada na própria tabela de dependências.
- Evidência histórica:
1. `zt_project_assign_value(...)` grava diretamente no campo final do manifesto;
2. não há rejeição explícita para a mesma chave aparecer duas vezes na mesma seção;
3. o valor posterior tende a sobrescrever o anterior sem diagnóstico dedicado.
- Arquivos principais:
1. `compiler/project/ztproj.c`
2. `tests/driver/test_project.c`
- Risco real:
1. configuração ambígua e difícil de depurar;
2. comportamento silencioso em revisão de manifestos;
3. tooling e documentação podem assumir uma semântica diferente da real.
- O que corrigir:
1. escolher política oficial: rejeitar duplicata ou aceitar apenas com regra explícita;
2. preferencialmente, rejeitar com código de erro dedicado.
- Plano de implementação:
1. rastrear chaves já vistas por seção durante o parse;
2. emitir diagnóstico claro ao repetir uma chave escalar;
3. tratar dependências separadamente, para não misturar tabela de dependências com chaves escalares;
4. adicionar fixture de manifesto duplicado ao gate do driver.
- Testes mínimos:
1. duplicata em `[project]`;
2. duplicata em `[build]`;
3. duplicata em `[app]`;
4. dependência repetida com política documentada.

### PLI-07 — Driver ainda depende de buffers fixos para descoberta e manipulação de paths
- Prioridade: `P3`
- Área: `Driver / Portabilidade`
- Status: `Corrigido` (2026-04-23)
- Resolução:
1. `zt_find_project_root_from_cwd(...)` em `compiler/driver/paths.c` foi reescrito para alocar `current`, `parent` e `manifest_path` via `malloc`, com piso mínimo `ZT_PATH_SCRATCH_FLOOR = 1024` e crescimento sob demanda para a `zenith.ztproj`. O guard de profundidade agora é a constante nomeada `ZT_PATH_MAX_ANCESTOR_STEPS = 512` com comentário explicativo.
2. `zt_make_dirs(...)` substituiu o `char buffer[1024]` por `malloc(length + 1)` dimensionado ao path exato.
3. `zt_project_source_file_list_push(...)` em `compiler/driver/project.c` agora aloca `normalized` dinamicamente, com cleanup via `goto done`.
4. `zt_project_discover_zt_files(...)` substituiu `pattern[768]` e `child_path[768]` em ambos os branches (`_WIN32`/POSIX) pelo novo helper `zt_join_path_alloc(...)` que retorna buffer dimensionado ao path real.
5. Casos legados com paths longos em monorepo/Windows long-path agora funcionam sem falha artificial de "path is too long".
- Evidência histórica:
1. `zt_find_project_root_from_cwd(...)` ainda usa buffers locais de `512/768` bytes;
2. a busca pelo root ainda usa guard de `256` níveis;
3. listas e normalização de path em partes do driver ainda assumem comprimentos fixos.
- Arquivos principais:
1. `compiler/driver/paths.c`
2. `compiler/driver/project.c`
- Risco real:
1. falha em árvores muito profundas;
2. limite artificial para paths longos, especialmente em Windows;
3. comportamento inconsistente entre ambientes e monorepos maiores.
- O que corrigir:
1. reduzir dependência de buffers fixos em operações de path do driver;
2. tornar o limite estrutural explícito, ou substituí-lo por abordagem mais robusta.
- Plano de implementação:
1. revisar `zt_find_project_root_from_cwd(...)` para operar com buffers dimensionados pelo `capacity` recebido ou caminho dinâmico;
2. trocar `512/768` por helpers centralizados de path-safe;
3. documentar o guard de profundidade, ou rebaixá-lo para hard cap claramente diagnosticado;
4. adicionar testes com paths longos e árvore profunda sintética.
- Testes mínimos:
1. descoberta de projeto em árvore profunda;
2. manifesto em path longo;
3. lista de arquivos com normalização em path próximo ao limite.

### PLI-08 — Canal de diagnostics do lexer é dead code e strings mal-fechadas não sinalizam erro
- Prioridade: `P2`
- Área: `Frontend / Diagnostics`
- Status: `Corrigido` (2026-04-23)
- Resolução:
1. Novo enum `ZT_DIAG_LEXER_UNTERMINATED_STRING` registrado em `compiler/semantic/diagnostics/diagnostics.h` com código estável `lexer.unterminated_string` (help: `Close the string literal with a matching delimiter.`).
2. Novo helper `zt_lexer_emit_diag` em `compiler/frontend/lexer/lexer.c` roteia mensagens para `zt_lexer_set_diagnostics(...)`.
3. Todas as três rotinas de leitura de literal (string, resume_string, triple_quoted) agora retornam `ZT_TOKEN_LEX_ERROR` quando encontram `\0`/`\n`/EOF sem o delimitador de fechamento, emitindo o diagnóstico estruturado.
4. `zt_parse` em `compiler/frontend/parser/parser.c` agora conecta o lexer principal ao canal via `zt_lexer_set_diagnostics(lexer, &result.diagnostics)` — antes apenas o sub-lexer de `fmt "..."` estava conectado.
5. Bug arquitetural correlato descoberto e corrigido: duas cópias divergentes de `diagnostics.h` (em `compiler/utils/` e `compiler/semantic/diagnostics/`) compartilhavam o mesmo include guard e causavam divergência silenciosa do enum dependendo da ordem de include. O arquivo em `compiler/utils/diagnostics.h` virou um wrapper fino que reexporta o header canônico.
- Evidência histórica:
1. `zt_lexer` mantém o campo `zt_diag_list *diagnostics` e expõe `zt_lexer_set_diagnostics(...)`, mas nenhum caminho do lexer emite diagnóstico nesse canal.
2. `zt_lexer_read_string(...)`, `zt_lexer_resume_string(...)` e `zt_lexer_read_triple_quoted(...)` saem do loop em `\0`/`\n`/EOF sem o delimitador de fechamento e ainda assim retornam `ZT_TOKEN_STRING_LITERAL`/`ZT_TOKEN_STRING_END`/`ZT_TOKEN_TRIPLE_QUOTED_TEXT` — como se o literal tivesse fechado corretamente.
3. O caminho é silencioso: nem `ZT_TOKEN_LEX_ERROR` é emitido, nem um diagnóstico estruturado entra na fila.
- Arquivo principal: `compiler/frontend/lexer/lexer.c`
- Risco real:
1. erros de digitação em literais de texto propagam para o parser como token "válido", produzindo mensagens fora do ponto real do erro;
2. input malicioso ou corrompido passa pelo lexer sem sinal claro, comprometendo confiança de fuzz/tooling;
3. o canal de diagnostics do lexer fica sem cobertura, cimentando dead code que será copiado em novas features.
- O que corrigir:
1. quando a leitura de literal terminar sem delimitador, emitir token `ZT_TOKEN_LEX_ERROR` e/ou um diagnóstico estável (por exemplo `lexer.unterminated_string`);
2. consolidar o uso do canal `zt_lexer_set_diagnostics` para esses casos;
3. alinhar com PLI-01 (limite) e PLI-05 (token_too_long) para evitar múltiplas fontes de verdade.
- Plano de implementação:
1. criar helper interno `zt_lexer_emit_diag(...)` para evitar duplicação;
2. substituir os caminhos "break em EOF" por emissão de diagnóstico + token de erro;
3. acrescentar entrada no catálogo de diagnósticos se ainda não existir código dedicado para `lexer.unterminated_string`;
4. documentar na spec de diagnostics que literais inválidos são reportados no lexer.
- Testes mínimos:
1. string simples sem fechamento;
2. triple-quoted sem fechamento;
3. string com interpolação parcial (`"hello {x"`);
4. caso válido, garantindo que diagnostics permanece vazio.

### PLI-09 — Buffers fixos de 64 bytes em partes de símbolo do emitter C truncam silenciosamente
- Prioridade: `P3`
- Área: `Backend C / Corretude`
- Status: `Corrigido` (2026-04-23)
- Resolução:
1. Constante central `C_EMIT_SYMBOL_PART_MAX = 256` introduzida em `compiler/targets/c/emitter.c` com comentário explicando o porquê.
2. Todos os `char module_name[64]`, `char function_name[64]`, `char struct_name[64]`, `char enum_name[64]`, `char variant_member[64]`, `char field_name[64]` e `char sanitized[64]` passaram a usar `C_EMIT_SYMBOL_PART_MAX`.
3. `variant_tag[256]` cresceu para `C_EMIT_SYMBOL_PART_MAX + 128 + 4` para acomodar `enum_symbol + "__" + variant_member` sem gerar warning de `-Wformat-truncation`.
4. Em combinação com `PLI-02`, mesmo que um namespace gigante ainda trunque em 255 bytes, a colisão resultante é agora detectada e reportada com diagnóstico claro.
- Evidência histórica:
1. Múltiplas funções em `compiler/targets/c/emitter.c` declaram buffers locais de `64` bytes para partes de nome antes da concatenação final (`char module_name[64]`, `char function_name[64]`, `char struct_name[64]`, `char enum_name[64]`, `char variant_member[64]`, `char field_name[64]`, `char sanitized[64]`).
2. Esses buffers são alimentados por `c_copy_sanitized(...)`, que trunca quando `out_index + 1 >= capacity` sem reportar erro.
3. Um namespace ou símbolo com mais de 63 bytes (após sanitização) colide com qualquer outro que compartilhe o mesmo prefixo de 63 bytes, independentemente da política de mapeamento caractere-a-caractere discutida em `PLI-02`.
- Arquivos principais:
1. `compiler/targets/c/emitter.c` (pontos típicos: `c_build_function_symbol`, `c_build_struct_symbol`, `c_build_enum_symbol`, `c_build_block_label`, `c_build_module_var_init_symbol`, `c_build_module_var_init_guard_symbol`, emissão de variantes de enum)
- Risco real:
1. colisão silenciosa entre módulos/funções/structs de projetos reais com nomes longos;
2. geração incorreta de código C mesmo quando `PLI-02` for resolvido;
3. regressão sutil e difícil de rastrear em monorepos com namespaces profundos.
- O que corrigir:
1. transformar as partes de símbolo em saída dimensionada, ou falhar explicitamente quando o limite for atingido;
2. alinhar com a política definitiva de `PLI-02` para encoding reversível de nomes.
- Plano de implementação:
1. introduzir helper central `c_emit_symbol_part(...)` que escreva em `char *` alocado (arena/scratch) com capacidade suficiente;
2. ou marcar truncamento explicitamente como erro de backend (`ZT_DIAG_BACKEND_C_EMIT_ERROR` com código `backend.c.symbol_truncated`);
3. adicionar tabela temporária de símbolos emitidos (compartilhada com `PLI-02`) para detectar colisão residual;
4. ajustar goldens para incluir casos com namespace longo.
- Testes mínimos:
1. função em namespace longo (>= 64 bytes após sanitização);
2. dois namespaces distintos que compartilhem o prefixo de 63 bytes;
3. struct e enum com nomes longos no mesmo módulo;
4. caso curto normal, garantindo que não há regressão de nome gerado.

### PLI-10 — Cinco casos do formatter não satisfazem `fmt(fmt(x)) == fmt(x)`
- Prioridade: `P2`
- Área: `Formatter / Correção`
- Status: `Corrigido` (2026-04-23)
- Descoberto por: gate de idempotência implementado em `PLI-04`
- Resolução (seis bugs distintos identificados e corrigidos):
1. **String literals sem aspas** em `compiler/tooling/formatter.c::sb_append_string_value` — quando a string não tinha chars de escape, o formatter omitia as aspas, fazendo `"Ada"` virar `Ada` (identificador). Removido o short-circuit: strings sempre emitidas entre aspas.
2. **Type params `[T]` em vez de `<T>`** em declarações de `func`, `struct`, `trait`, `apply`, `enum` — o formatter emitia `[T]` mas o parser só aceita `<T>`. Trocado para alinhar com a spec e gramática canonical.
3. **`end` espúrio em `match case`** — `ZT_AST_MATCH_CASE` chamava `format_node(body)`, que emitia `\n...\nend`, criando um `end` por case além do `end` do `match`. Corrigido para iterar statements sem delimitador de bloco.
4. **Vírgula entre variants de enum** — formatter emitia `,\n` mas o parser não consome vírgulas entre variants, causando erro de re-parse. Removido; variants separadas apenas por newline.
5. **`where NAME -> COND`** em `WHERE_CLAUSE` — o parser lê apenas `where EXPR` (o `param_name` é recuperado do contexto). O `NAME -> ` espúrio era rejeitado pelo parser. Removido.
6. **Use-after-free de `source_text`** em `compiler/driver/project.c::zt_parse_project_sources` — o buffer era liberado logo após `zt_parse`, mas os tokens de comentário guardavam ponteiros para essa memória. `fmt` silenciosamente emitia lixo (ex: `-- c` virando `solat`), fazendo o arquivo truncar na segunda passada. Removido o `free` precoce; o dispose no fim do ciclo já libera corretamente.
7. **Comment "stealing" em cascata no parser** — `ast_make` em `compiler/frontend/parser/parser.c` anexava todos os `pending_comments` a qualquer nó criado, fazendo:
   - comentários de declaração serem roubados pelo primeiro `TYPE_SIMPLE` interno (`func foo(value: -- comment int)`);
   - comentários "trailing" (entre fim de um nó e início do próximo) serem absorvidos pelo nó atual, gerando oscilação entre passes.
   Corrigido com duas mudanças:
   - `zt_ast_kind_accepts_leading_comments` — whitelist que só permite comentários em nós de declaração/statement/field/variant/case, evitando que TYPE/EXPR/BLOCK os capturem.
   - Padrão `stash_leading` + `reclaim_node_trailing` + `apply_stash_to_node` aplicado nos loops de `zt_parse` (imports + declarations) e `parse_block_ex` (statements), isolando cada nó sibling em seu próprio escopo de comentários.
8. **Goldens atualizados** em `tests/formatter/cases/{case_all,case_imports,case_match,case_generics,case_comments,case_structs,case_triple_quoted,case_trailing_commas,case_manifest}/expected/` para refletir a saída canônica após as correções.
9. Todos os 9 casos promovidos para `IDEMPOTENT_CASES` em `tests/formatter/run_formatter_idempotence.py`; `XFAIL_CASES` agora vazio.
- Evidência de correção:
1. `python tests/formatter/run_formatter_idempotence.py` → 9/9 `[OK  ]`.
2. `python run_suite.py pr_gate` → tooling pass=4/4, frontend pass=108/108, backend pass=6/6.
3. Formatter golden e idempotence ambos verdes.
- Arquivos afetados:
1. `compiler/tooling/formatter.c`
2. `compiler/frontend/parser/parser.c`
3. `compiler/driver/project.c`
4. `tests/formatter/run_formatter_idempotence.py`
5. `tests/formatter/cases/*/expected/`

---

## 3. Itens reavaliados e não reabertos

Os relatórios legados tinham vários achados válidos no momento em que foram escritos, mas que hoje não devem voltar como backlog ativo.

Eles ficaram em um destes grupos:

- `corrigido`;
- `reclassificado como decisão arquitetural`;
- `hardening contínuo, sem bug confirmado ativo`;
- `evidência insuficiente no código atual`.

Exemplos importantes já não abertos:

- overflow antigo em `hex bytes`;
- truncamento silencioso em namespace path;
- autoload de `import std.*` por comentário;
- `system()` no pipeline/process runner;
- ARC não atômico como “bug”, em vez de contrato do alpha;
- UTF-8 inválido entrando por `zt_text_from_utf8()` público;
- busca linear quente de catálogo;
- `map<text,text>` sem caminho hash real.

O que este arquivo agora também absorve:

- o que ainda restava de útil no antigo `reports/full-audit-report.md`;
- o que ainda restava de útil nos relatórios legados de 2026-04-22 já removidos.

---

## 4. Ordem recomendada

Não há pendências restantes das 10 originais.

Corrigidos nos lotes de 2026-04-23:

- `PLI-01` — limite estrutural no lexer
- `PLI-02` — detecção de colisão de símbolos no emitter C
- `PLI-03` — truncamento de mensagem de contrato
- `PLI-04` — gate de idempotência do formatter
- `PLI-05` — limite uniforme para tokens muito longos
- `PLI-06` — duplicatas silenciosas no manifesto
- `PLI-07` — buffers fixos de path no driver
- `PLI-08` — diagnostics ausente para literais mal-fechados no lexer
- `PLI-09` — buffers de 64 bytes em partes de símbolo do emitter
- `PLI-10` — cinco casos de não-idempotência do formatter (seis bugs distintos)

---

## 5. Fechamento

Este arquivo substitui, para fins de backlog corretivo residual, os relatórios legados reavaliados em 2026-04-23 e o antigo `full-audit-report.md`.

Se um item não estiver aqui, ele não deve voltar como pendência ativa sem nova revalidação no código atual.
