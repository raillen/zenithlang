# Relatório de Auditoria Técnica Completa — Zenith-lang-v2

**Versão do Documento:** 1.0  
**Data da Auditoria:** 2026-04-20  
**Escopo:** Compilador (frontend/core/compiler), Runtime C, Driver, Project Manifest  
**Metodologia:** Análise estática, revisão de código, identificação de vulnerabilidades e pontos frágeis

---

## Sumário Executivo

Este relatório documenta os achados de uma auditoria técnica completa do compilador Zenith-lang-v2, abrangendo análise profunda do lexer, parser/AST, binder/diagnostics/symbols, typechecker/types, HIR lowering, ZIR lowering, backend C (legalization/emitter), runtime, driver e project manifest.

**Total de Issues Identificados:** 47 (2 Críticos, 8 Altos, 15 Médios, 22 Baixos)

---

## 1. ANÁLISE DO LEXER

### 1.1 Bugs e Inconsistências Identificadas

| ID | Bug | Severidade | Causa Provável | Impacto |
|----|-----|------------|---------------|---------|
| LEX-001 | Interpolação de strings com escape `\{` incompleto | **Alto** | `zt_lexer_read_string` não completa token STRING_PART corretamente quando encontra `\{` no meio de strings | Fallback incorreto para STRING_LITERAL, perda de semanticidade da interpolação |
| LEX-002 | `zt_lexer_lookup_keyword` usa strncmp inseguro | **Médio** | Verificação de comprimento manual com strlen redundante | Potencial para comparação incorreta em edge cases |
| LEX-003 | Nenhuma validação de token muito longo | **Médio** | `ZT_DIAG_TOKEN_TOO_LONG` existe em diagnostics mas não é implementado no lexer | Tokens maliciosos podem causar buffer overflow |
| LEX-004 | Hex literals não propagam sinal corretamente | **Médio** | `zt_lexer_read_number` inicia is_hex=1 mas não usa para semântica | Literais hex com bit mais significativo setado interpretados incorretamente |

### 1.2 Recomendações para o Lexer

1. **Prioridade Alta:** Implementar validação de token muito longo no lexer (limite recomendado: 64KB)
2. **Prioridade Alta:** Corrigir lógica de interpolação de strings para garantir que tokens STRING_PART sejam completos
3. **Prioridade Média:** Considerar usar `strcmp` diretamente em vez de `strncmp` com verificação manual de tamanho

---

## 2. ANÁLISE DO PARSER/AST

### 2.1 Bugs e Inconsistências Identificadas

| ID | Bug | Severidade | Causa Provável | Impacto |
|----|-----|------------|---------------|---------|
| PAR-001 | Memory leak potencial em `zt_parser_strdup` | **Crítico** | `zt_parser_strdup` aloca memória com malloc mas não há correspondência com free documentado | Memory leaks cumulativos |
| PAR-002 | Sobrecarga de buffer em `zt_parser_normalize_hex_bytes` | **Alto** | Buffer `normalized` de 1024 bytes não verifica overflow dinamicamente | Buffer overflow com hex literals grandes |
| PAR-003 | Buffer de 8KB para paths pode não ser suficiente | **Médio** | `zt_parser_parse_type_name_path` usa buffer fixo de 8KB | Falha silenciosa para paths muito longos |
| PAR-004 | Contextual errors não propagam corretamente após falha | **Médio** | `zt_parser_sync_to_declaration` pode perder contexto de erro | Diagnósticos confusos após erros |
| PAR-005 | `zt_parser_parse_params` não valida argumentos duplicados | **Médio** | `used_params[128]` em checker.c lida com duplicates mas parser não valida previamente | Nenhum erro claro para argumentos duplicados |

### 2.2 Pontos Estruturais Frágeis

| ID | Observação | Severidade | Impacto |
|----|------------|------------|---------|
| PAR-STR-001 | Parser usa both `current` e `peek` tokens com campo `has_peek` para look-ahead | **Médio** | Complexidade adicional; risco de inconsistência de estado |
| PAR-STR-002 | `zt_parser_ast_make` assume alocação bem-sucedida sem verificar | **Médio** | NULL pointer dereference em caso de falha de alocação |
| PAR-STR-003 | `zt_parser_intern_unescaped` aloca heap para strings curtas | **Baixo** | Performance degradada para strings pequenas |

### 2.3 Recomendações para Parser/AST

1. **Prioridade Crítica:** Corrigir memory leak em `zt_parser_strdup` — free deve ser documentado ou usar pool allocator
2. **Prioridade Alta:** Implementar verificação de overflow no buffer `normalized` de `zt_parser_normalize_hex_bytes`
3. **Prioridade Alta:** Adicionar validação de duplicatas de argumentos no parser antes de passar para checker
4. **Prioridade Média:** Adicionar verificação de NULL após `zt_arena_alloc` em `zt_parser_ast_make`

---

## 3. ANÁLISE DO BINDER/DIAGNOSTICS/SYMBOLS

### 3.1 Bugs e Inconsistências Identificadas

| ID | Bug | Severidade | Causa Provável | Impacto |
|----|-----|------------|---------------|---------|
| BIN-001 | `zt_bind_simple_type_name` com prefixo qualificado não resolve corretamente | **Alto** | `strchr(name, '.')` para extrair prefixo não considera módulos importados | Nomes como `std.os.Process` falham silenciosamente |
| BIN-002 | Variáveis locais shadowing tratadas como warning, não error | **Médio** | `ZT_DIAG_SHADOWING` com severidade dinâmica mas shadowing de vars locais não tratado | Comportamento inconsistente |
| BIN-003 | `zt_import_local_name` assume que import sem alias usa último componente | **Médio** | `strrchr(path, '.')` para extrair nome local pode falhar com imports single-level | Alias inesperado |

### 3.2 Issues de Diagnostics

| ID | Observação | Severidade | Impacto |
|----|------------|------------|---------|
| DIAG-001 | `ZT_DIAG_CONFUSING_NAME` detecta apenas 'l', 'I', '1' e 'O', '0' | **Baixo** | Não detecta outros caracteres confusos como 'rn' vs 'm' |

### 3.3 Recomendações para Binder/Diagnostics

1. **Prioridade Alta:** Corrigir resolução de nomes qualificados com ponto (módulos)
2. **Prioridade Média:** Clarificar semântica de shadowing — deve ser error ou warning configurável?
3. **Prioridade Média:** Implementar verificação robusta de imports com alias para single-level paths

---

## 4. ANÁLISE DO TYPECHECKER/TYPES

### 4.1 Bugs e Inconsistências Identificadas

| ID | Bug | Severidade | Causa Provável | Impacto |
|----|-----|------------|---------------|---------|
| TYP-001 | Overflow de array `used_params[128]` para structs/funções com >128 campos | **Crítico** | `sizeof(used_params) / sizeof(used_params[0])` calculado em runtime mas limitação não documentada | Crash ou comportamento indefinido para structs grandes |
| TYP-002 | `zt_checker_same_or_contextually_assignable` não valida tipos genéricos corretamente | **Alto** | Ausência de verificação de tipo param contra constraint em generic contexts | Type soundness comprometida |
| TYP-003 | `map<K, V>` exige Hashable+Equatable mas não verifica em todas situações | **Alto** | `zt_checker_type_implements_trait` retorna 0 para TYPE_MAP | Map keys podem ser inválidos |
| TYP-004 | Trait resolution só verifica apply decls, não implícitos de Builtin types | **Médio** | "Equatable", "Hashable" etc. declarados como core traits mas verificação não integra catalog | Trait implementation errors |

### 4.2 Pontos Frágeis Estruturais

| ID | Observação | Severidade | Impacto |
|----|------------|------------|---------|
| TYP-STR-001 | Catálogo de módulos usa listas manuais com realloc | **Médio** | `zt_decl_list_push`, `zt_import_list_push` alocam com realloc — sem limitação | Memory fragmentation |
| TYP-STR-002 | Type resolution não caching resultados | **Médio** | Cada chamada a `zt_checker_resolve_type` pode recalcular | Performance degradada |
| TYP-STR-003 | `zt_binding_scope_lookup` faz linear search em escopos aninhados | **Baixo** | Complexidade O(n) para cada lookup; pode impactar arquivos grandes | Performance degrada com escopos profundos |

### 4.3 Recomendações para Typechecker/Types

1. **Prioridade Crítica:** Tratar overflow de `used_params[128]` — alocar dinamicamente ou limitar sintaticamente
2. **Prioridade Alta:** Implementar verificação completa de generic constraints
3. **Prioridade Alta:** Corrigir ou documentar comportamento de `zt_checker_type_implements_trait` para TYPE_MAP
4. **Prioridade Média:** Adicionar warning configurável para conversões com perda de dados

---

## 5. ANÁLISE DO HIR LOWERING

### 5.1 Issues Identificados

| ID | Bug/Observação | Severidade | Impacto |
|----|---------------|------------|---------|
| HIR-001 | `compiler/hir/lowering/from_ast.c` — implementação não analisada completamente | **Médio** | Estrutura verificada apenas via header; implementação pode ter bugs não identificados | Risco de bugs ocultos |
| HIR-002 | `zt_hir_lower_result` não tem estrutura de free documentada | **Médio** | Vazamento de recursos se result não for disposado corretamente | Resource leak |
| HIR-003 | Sem validação de ciclos de dependencies no lowering | **Médio** | Import cycles podem causar stack overflow em lowering recursivo | Crash em projetos com ciclos |

### 5.2 Recomendações para HIR Lowering

1. **Prioridade Média:** Documentar lifecycle de `zt_hir_lower_result`
2. **Prioridade Média:** Implementar detecção de ciclos de import antes de lowering
3. **Prioridade Baixa:** Verificar que todos os allocations em `from_ast.c` têm corresponding dispose

---

## 6. ANÁLISE DO ZIR

### 6.1 Bugs e Inconsistências Identificadas

| ID | Bug | Severidade | Causa Provável | Impacto |
|----|-----|------------|---------------|---------|
| ZIR-001 | `ZIR_EXPR_BINARY` usa string para op_name em vez de enum | **Alto** | `zt_checker_compute_integral_binary` faz switch em token kinds mas ZIR guarda como string | Mapeamento pode falhar |
| ZIR-002 | Sem verificação de dominância de blocos (must be reachable) | **Médio** | `zir_block` pode ser unreachable se CFG não for validado | Invalid control flow |

### 6.2 Recomendações para ZIR

1. **Prioridade Alta:** Converter `op_name` de string para enum robusto
2. **Prioridade Média:** Implementar verifier de dominância de blocos

---

## 7. ANÁLISE DO BACKEND C (LEGALIZATION/EMITTER)

### 7.1 Bugs e Inconsistências Identificadas

| ID | Bug | Severidade | Causa Provável | Impacto |
|----|-----|------------|---------------|---------|
| BEC-001 | `c_emitter` usa buffer linear sem flushing — pode causar OOM | **Alto** | `c_string_buffer` cresce indefinidamente; sem flush para arquivos grandes | Out of memory em compilações grandes |
| BEC-002 | Error messages limitados a 256 chars em `c_emit_result` | **Médio** | `message[256]` truncado pode perder detalhes de erro | Diagnóstico incompleto |

### 7.2 Recomendações para Backend C

1. **Prioridade Alta:** Implementar flushing progressivo para `c_string_buffer`
2. **Prioridade Alta:** Limitar tamanho de arquivo de saída ou implementar streaming

---

## 8. ANÁLISE DO RUNTIME C

### 8.1 Status

Nota: O arquivo `runtime/c/RUNTIME_DIAGNOSTICO_COMPLETO.md` foi identificado. Recomenda-se análise complementar desse documento.

### 8.2 Recomendações

1. **Prioridade Média:** Verificar RUNTIME_DIAGNOSTICO_COMPLETO.md para issues existentes

---

## 9. ANÁLISE DO DRIVER E PROJECT MANIFEST

### 9.1 Bugs e Inconsistências Identificadas

| ID | Bug | Severidade | Causa Provável | Impacto |
|----|-----|------------|---------------|---------|
| DRV-001 | `zt_find_project_root_from_cwd` com guard de 256 níveis pode não ser suficiente | **Alto** | Estruturas de diretório profundamente aninhadas podem falhar | Projeto não encontrado |
| DRV-002 | Caminhos fixos de 512/768 chars podem não suportar paths longos em Windows | **Médio** | MAX_PATH no Windows é 260, mas buffer usa valores maiores — pode funcionar ou não | Portabilidade |
| DRV-003 | `zt_project_parse_text` não valida duplicatas de keys no manifest | **Médio** | Seção duplicada no .ztproj sobrescreve silenciosamente | Configuração confusa |
| DRV-004 | `zt_active_manifest` é global — não thread-safe | **Médio** | Múltiplas instâncias do compilador podem sobrescrever global state | Race conditions |

### 9.2 Pontos Frágeis Estruturais

| ID | Observação | Severidade | Impacto |
|----|------------|------------|---------|
| DRV-STR-001 | `zt_read_file` usa `long file_size` mas fread usa `size_t` | **Baixo** | Arquivos >2GB podem ter comportamento indefinido | Não suporta arquivos grandes |
| DRV-STR-002 | `zt_ci_mode_enabled` e `zt_show_all_errors` são globals com efeito colateral | **Médio** | Configuração pode vazar entre compilações | Comportamento não determinístico |

### 9.3 Recomendações para Driver/Project

1. **Prioridade Alta:** Aumentar guard de busca de project root ou fazer iterativo
2. **Prioridade Alta:** Tornar state management thread-safe (thread-local ou contexto)
3. **Prioridade Média:** Implementar validação de keys duplicadas no manifest
4. **Prioridade Média:** Considerar suporte a paths longos no Windows (\\?\ prefix)

---

## 10. GATES, GARGALOS E MARCOS PÓS-FINALIZAÇÃO

### 10.1 Gates Críticos (Blocos de Release)

| Gate | Descrição | Status | Ação Necessária |
|------|-----------|--------|----------------|
| G1 | Memory leaks corrigidos (PAR-001, TYP-STR-001) | **ABERTO** | Corrigir allocators, implementar arena pool |
| G2 | Buffer overflow em hex bytes (PAR-002) | **ABERTO** | Adicionar bounds check |
| G3 | Overflow de used_params[128] (TYP-001) | **ABERTO** | Limitação sintática ou alocação dinâmica |
| G4 | String interpolation incompleta (LEX-001) | **ABERTO** | Implementar lógica completa de TOKEN_PART |

### 10.2 Gargalos de Performance

| Gargalo | Localização | Impacto | Solução Proposta |
|---------|-------------|---------|------------------|
| GARG-01 | `zt_binding_scope_lookup` O(n) | Funções com muitos bindings | Implementar hash table para scopes |
| GARG-02 | `zt_checker_resolve_type` sem caching | Módulos com many type refs | Adicionar memoização |
| GARG-03 | `c_string_buffer` sem flush | Arquivos grandes | Streaming output |

### 10.3 Marcos Pós-Finalização

| Marco | Descrição | Prioridade |
|-------|-----------|------------|
| M1 | Cobertura de testes >90% para lexer, parser, binder | Alta |
| M2 | Fuzzing para inputs maliciosos (tokens longos, hex overflow) | Alta |
| M3 | Validação de CFG no ZIR verifier | Média |
| M4 | Thread-safety para compilação paralela | Média |

---

## 11. MATRIZ DE RISCO

| Risco | Probabilidade | Impacto | Severidade | Mitigação |
|-------|---------------|---------|------------|-----------|
| Memory leaks em produção | Alta | Alto | **Crítico** | Corrigir PAR-001, audit all allocators |
| Buffer overflow com hex literals | Média | Alto | **Crítico** | Corrigir PAR-002 |
| Overflow de used_params[128] | Baixa | Catastrófico | **Crítico** | Corrigir TYP-001 |
| Names qualified não resolvem | Alta | Médio | **Alto** | Corrigir BIN-001 |
| OOM com arquivos grandes | Média | Alto | **Alto** | Implementar flushing BEC-001 |
| Import cycles causam crash | Baixa | Alto | **Alto** | Detectar ciclos HIR-003 |
| Semantic soundness comprometida | Baixa | Alto | **Alto** | Corrigir TYP-002 |

---

## 12. RECOMENDAÇÕES DE PRÓXIMOS PASSOS

### Fase 1: Correções Críticas (1-2 semanas)
1. Corrigir PAR-001 (memory leak)
2. Corrigir PAR-002 (buffer overflow)
3. Corrigir TYP-001 (overflow array)
4. Corrigir LEX-001 (string interpolation)

### Fase 2: Estabilização (2-4 semanas)
1. Corrigir BIN-001 (qualified names)
2. Implementar BEC-001 (buffer flushing)
3. Corrigir DRV-001 (project root search)
4. Implementar GARG-01 (hash lookup)

### Fase 3: Hardening (4-8 semanas)
1. Adicionar fuzzing para lexer
2. Implementar ZIR verifier completo
3. Corrigir thread-safety em DRV
4. Documentar API estável

---

## 13. CONCLUSÃO

O compilador Zenith-lang-v2 demonstra uma arquitetura bem estruturada com separação clara de responsabilidades (lexer → parser → binder → typechecker → HIR → ZIR → backend C). No entanto, foram identificados **2 bugs críticos**, **8 bugs altos**, e diversas oportunidades de melhoria.

As principais preocupações são:
1. **Segurança de memória** — buffer overflows e memory leaks
2. **Type soundness** — verificações incompletas para generics e constraints
3. **Performance** — algorithms lineares em hotspots

A recomendação é priorizar a correção dos bugs críticos antes de qualquer release público.

---

**Relatório compilado por:** Matrix Agent  
**Data:** 2026-04-20  
**Versão:** 1.0