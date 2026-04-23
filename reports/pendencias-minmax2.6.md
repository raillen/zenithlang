# Débitos Técnicos Identificados — Análise Minimax-M2.6

**Data:** 22-04-2026  
**Baseado em:** Relatórios de gemma4-31b, qwen3.5-plus, qwen3.6-plus  
**Versão Alvo:** 0.3.0-alpha.1 (R2.M12)  
**Status:** Compilação de débitos técnicos confirmados no código atual

---

## 1. Bugs CRÍTICOS (Crash, Memory Corruption, Undefined Behavior)

| ID | Componente | Bug | Arquivo:Linha | Detalhes |
|----|-----------|-----|---------------|----------|
| **LEX-01** | Lexer | String literal ilimitada | `compiler/frontend/lexer/lexer.c:168` | `zt_lexer_read_string()` lê até `"` ou EOF sem limite. Strings não fechadas consomem memória até OOM. |
| **RUN-01** | Runtime | RC não-atômico | `runtime/c/zenith_rt.c:1455, 1473` | `zt_retain()`/`zt_release()` usam `header->rc += 1` sem operações atômicas. Race condition em threads via FFI causa double-free ou leak. |
| **RUN-02** | Runtime | NULL deref em deep copy | `runtime/c/zenith_rt.c:1587` | `zt_deep_copy()` para `list<text>` não verifica se `zt_deep_copy(l->data[i])` retorna NULL após OOM. Armazena NULL em `clone->data[i]` → crash. |
| **RUN-03** | Runtime | Leak em falha de alloc | `runtime/c/zenith_rt.c:2042-2045` | `zt_text_from_utf8()` aloca `value` (linha 2036) e depois `value->data` (linha 2042). Se segundo malloc falhar, `value` é free'd mas não há unwind do primeiro. |
| **SEM-03** | Checker | Signed overflow UB | `compiler/semantic/types/checker.c:537` | `zt_checker_compute_integral_binary()` faz `*out_value = left + right` em `long long` sem check de overflow. `9223372036854775807 + 1` é UB em C. |
| **PIP-01** | Pipeline | Buffer truncation + injection | `compiler/driver/pipeline.c:1112, 1172` | `compile_cmd[2048]` pode truncar com caminhos longos + `linker_flags`. command injection via `build.linker_flags`. |

---

## 2. Bugs de ALTA Severidade (Incorrect Behavior in Edge Cases)

| ID | Componente | Bug | Arquivo:Linha | Detalhes |
|----|-----------|-----|---------------|----------|
| **RUN-09** | Runtime | UTF-8 validation não chamada | `runtime/c/zenith_rt.c:2032` | `zt_text_from_utf8()` NÃO chama `zt_utf8_validate()`. Sequências overlong, surrogates, bytes de continuação inválidos são criados como `text` válido. |
| **STD-03** | Runtime | Path traversal em read_file | `runtime/c/zenith_rt.c:3896` | `zt_host_default_read_file()` aceita `../../../etc/passwd` sem validar `..`. Leitura de arquivos arbitrários no filesystem host. |
| **RUN-05** | Runtime | Capacidade fixa mapa=8 | `runtime/c/zenith_rt_templates.h` | Hash map open-addressing com capacidade inicial fixa em 8. Sem rehash strategy. Degrada O(n) para >1000 entries. |
| **RUN-06** | Runtime | size_t overflow em list growth | `runtime/c/zenith_rt_templates.h` | `new_capacity *= 2` pode overflow `size_t` para listas com > SIZE_MAX/2 elementos → alocação de buffer pequeno → heap corruption. |
| **EMI-02** | Emitter | strncpy sem null-term | `compiler/targets/c/emitter.c:48` | `c_canonicalize_type()` usa `strncpy(dest, src, capacity)` sem garantir `dest[capacity-1] = '\0'`. Tipo >= 128 chars causa UB. |
| **EMI-03** | Emitter | Colisão de símbolos (- vs _) | `compiler/targets/c/emitter.c:231` | `c_copy_sanitized()` mapeia `-` e `_` ambos para `_`. `app.main-test` e `app.main_test` colidem para `app_main_test`. |
| **EMI-04** | Emitter | Whitespace não deduplicado em monomorphization | `compiler/targets/c/emitter.c` | `list<list<int>>` vs `list<list< int>>` geram instâncias separadas. Diferenças de espaços em tipos generic são treatados como tipos distintos. |
| **SEM-05** | Checker | O(n) catalog lookup | `compiler/semantic/types/checker.c:237` | `zt_catalog_find_decl()` faz scan linear O(n) pelos declarações. Proyectos com 2000+ símbolos mostram degradação O(n²). |
| **RUN-07** | Runtime | Slice normalization inconsistente | `runtime/c/zenith_rt.c:493` | `zt_normalize_slice_end()` comporta-se de forma inconsistente para containers vazios. `[0..-1]` em text vazio vs list vazia produz resultados diferentes. |
| **RUN-08** | Runtime | Buffer 512 bytes em contract | `runtime/c/zenith_rt.c:1418` | Mensagens de contract failure usam buffer de 512 chars. Mensagens > 512 chars truncam silenciosamente. |

---

## 3. Bugs MÉDIOS (Degradation, Leaks, Minor Risks)

| ID | Componente | Bug | Arquivo:Linha | Detalhes |
|----|-----------|-----|---------------|----------|
| **PIP-02** | Pipeline | Falso positivo scan stdlib | `compiler/driver/pipeline.c:549` | Scan de imports usa `strstr(line, "import std.")`. Comentários como `-- import std.io` disparam carregamento desnecessário. |
| **SEM-01** | Checker | Use-after-free em scope dispose | `compiler/semantic/binder/binder.c` | `zt_binding_scope_dispose()` libera memória mas não invalida ponteiros. Reuso de scopes disposed causa use-after-free. |
| **STD-04** | Stdlib | Stack overflow em JSON parser | `runtime/c/zenith_rt.c:3901` | `zt_json_parse_map_text_text()` usa descida recursiva sem limite de profundidade. JSON com 10k+ níveis causa stack overflow. |
| **STD-02** | Runtime | Double-close socket | `runtime/c/zenith_rt.c` | `zt_net_connection` permite `close()` duplo do socket fd. Segundo close pode fechar fd errado se houve realocação. |

---

## 4. Bugs de BAIXA Prioridade (Melhorias Desejáveis)

| ID | Componente | Bug | Descrição |
|----|-----------|-----|-----------|
| **FMT-01** | Formatter | Sem garantia de idempotência | `format(format(x))` pode não ser igual a `format(x)`. Não há teste para idempotência do formatter. |
| **SEM-06** | Checker | Float-to-int truncation sem aviso | Float literal grande atribuído a int não gera warning. Truncation silencioso. |
| **PIP-03** | Pipeline | Sem file lock em build paralelo | Escrita em `.ztc-tmp/runtime/zenith_rt.o` sem locking. Builds paralelos podem corromper objeto. |
| **PIP-04** | Pipeline | Unknown manifest fields ignorados | Campos desconhecidos em `zenith.ztproj` são silenciosamente descartados. Typos não são pegos. |

---

## 5. Resumo Estatístico

| Severidade | Quantidade |
|------------|------------|
| CRÍTICO | 6 |
| ALTO | 10 |
| MÉDIO | 4 |
| BAIXO | 4 |
| **TOTAL** | **24** |

---

## 6. Priorização para Correção

### Fase 1 — Correções Imediatas (Segurança/Crash)
1. **RUN-01** — Tornar `zt_retain()`/`zt_release()` atômicos (ou documentar como não-thread-safe)
2. **PIP-01** — Sanitizar `linker_flags` ou usar `execvp` em vez de `system()`
3. **RUN-02** — Adicionar check de NULL em `zt_deep_copy()` para `list<text>`
4. **STD-03** — Adicionar validação de path (rejeitar segmentos `..`)

### Fase 2 — Correções de Corretude
5. **SEM-03** — Adicionar overflow check em constant folding
6. **RUN-09** — Chamar `zt_utf8_validate()` em `zt_text_from_utf8()`
7. **RUN-03** — Corrigir leak em `zt_text_from_utf8()` no segundo malloc fail
8. **LEX-01** — Adicionar limite de tamanho em `zt_lexer_read_string()`

### Fase 3 — Melhorias de Performance
9. **RUN-05** — Implementar rehash strategy para maps
10. **SEM-05** — Substituir O(n) lookup por hash table ou árvore balanceada

---

*Gerado por minimax-m2.6 em 22-04-2026 via análise comparativa dos relatórios de gemma4-31b, qwen3.5-plus e qwen3.6-plus contra o código fonte atual.*
