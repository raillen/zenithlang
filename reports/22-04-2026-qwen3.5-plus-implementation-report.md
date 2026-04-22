# Heavy Tests — Relatório de Implementação

**Data:** 2026-04-22
**Versão Zenith:** 0.3.0-alpha.1 (R2.M12)
**Status:** ✅ Implementado

---

## 📊 Resumo Executivo

Foram implementados **13 testes automatizados** cobrindo 6 categorias críticas, mais um fuzzer de semantic layer com 18 geradores. Os testes visam encontrar bugs reais de segurança, crash, performance e corretude.

---

## 🗂️ Estrutura Criada

```
tests/heavy/
├── README.md                      # Documentação principal
├── run_heavy_tests.py             # Runner principal
├── reports/                       # Relatórios de execução
├── security/                      # Security vulnerability tests
│   ├── buffer_overflow/
│   │   ├── hex_bytes_overflow/    # LEX-02: 600+ hex pairs
│   │   └── params_overflow/       # SEM-01: 150 parâmetros
│   ├── command_injection/         # RUN-05: std.os.process.run RCE
│   ├── path_traversal/            # STD-03: ../../../etc/passwd
│   └── race_conditions/
│       └── rc_race/               # RUN-01: RC não-atômico
├── crash/
│   ├── utf8_invalid/              # RUN-10: Overlong UTF-8
│   └── integer_overflow/          # SEM-03: i64 signed overflow
├── fuzz/
│   └── semantic/
│       ├── fuzz_semantic.py       # Fuzzer com 18 geradores
│       ├── crashes/               # Casos de crash salvos
│       └── hangs/                 # Casos de hang salvos
├── stress/
│   ├── nesting/
│   │   └── deep_500/              # 500 níveis de if/else
│   └── loc/
│       └── large_10k/             # 10k+ linhas
├── correctness/
│   ├── qualified_names/           # FUNC-01: app.utils.helper
│   └── generics/
│       └── map_constraints/       # FUNC-03: map<Unhashable,int>
└── performance/
    └── maps/
        └── map_perf/              # RUN-05: 10k inserts <5s
```

---

## 🧪 Testes Implementados

### Security Tests (5 testes)

| ID | Teste | Bug Alvo | Arquivo | Severidade |
|----|-------|----------|---------|------------|
| SEC-01 | `buffer_overflow_hex` | LEX-02: Hex bytes >512 pares | `lexer.c:278-308` | 🔴 CRITICAL |
| SEC-02 | `buffer_overflow_params` | SEM-01: `used_params[128]` | `checker.c` | 🔴 CRITICAL |
| SEC-03 | `command_injection` | RUN-05: `system()` injection | `zenith_rt.c:3525` | 🔴 CRITICAL |
| SEC-04 | `path_traversal` | STD-03: `../` traversal | `zenith_rt.c` | 🟠 HIGH |
| SEC-05 | `rc_race_condition` | RUN-01: RC não-atômico | `zenith_rt.c:865-975` | 🔴 CRITICAL |

### Crash Tests (2 testes)

| ID | Teste | Bug Alvo | Arquivo | Severidade |
|----|-------|----------|---------|------------|
| CRS-01 | `utf8_invalid` | RUN-10: UTF-8 overlong | `zenith_rt.c:1526-1556` | 🟠 HIGH |
| CRS-02 | `integer_overflow` | SEM-03: Signed overflow UB | `checker.c:533-556` | 🟠 HIGH |

### Stress Tests (2 testes)

| ID | Teste | Métrica | Timeout |
|----|-------|---------|---------|
| STR-01 | `deep_nesting_500` | 500 níveis de if/else | 60s |
| STR-02 | `large_file_10k` | 10k+ linhas, 2k funções | 120s |

### Correctness Tests (2 testes)

| ID | Teste | Bug Alvo | Expected |
|----|-------|----------|----------|
| COR-01 | `qualified_names` | FUNC-01: `app.utils.helper` | Exit 42 |
| COR-02 | `generic_constraints` | FUNC-03: map requer Hashable | Check fail |

### Performance Tests (1 teste)

| ID | Teste | Métrica Alvo | Timeout |
|----|-------|--------------|---------|
| PRF-01 | `map_operations` | 10k inserts <5s | 120s |

---

## 🔬 Semantic Fuzzer

**Arquivo:** `tests/heavy/fuzz/semantic/fuzz_semantic.py`

### Geradores Implementados (18)

| Gerador | Descrição | Bug Potencial |
|---------|-----------|---------------|
| `type_mismatch` | Atribuição entre tipos incompatíveis | Type checker bypass |
| `circular_type` | Tipos circulares (NodeA ↔ NodeB) | Infinite loop no checker |
| `generic_overflow` | Tipos genéricos 50+ níveis | Stack overflow |
| `trait_violation` | map<Unhashable, int> | Constraint bypass |
| `mutability_violation` | Mutação de const struct | Memory corruption |
| `optional_abuse` | optional<int> + 10 | Type confusion |
| `result_propagation` | ? fora de contexto result | Crash no checker |
| `where_contract` | `where it > 10 and it < 5` | Contract panic |
| `self_reference` | Struct contém self não-opcional | Infinite size |
| `method_receiver` | const receiver, mut method | UB |
| `duplicate_symbol` | Duas funções mesmo nome | Symbol table crash |
| `unbound_type` | `NonExistentType` | Null deref |
| `array_overflow` | Struct com 150 campos | Buffer overflow |
| `deep_if` | 200 níveis de if/else | Stack overflow |
| `fmt_interpolation` | `fmt "Hello {{name"` | Parser crash |
| `hex_bytes_overflow` | 600+ hex pairs | Buffer overflow |
| `large_int_literal` | `9223372036854775808` | Overflow UB |
| `float_overflow` | `1e309` | Inf/UB |

### Execução

```bash
# 100 iterações, seed aleatório
python tests/heavy/fuzz/semantic/fuzz_semantic.py --iterations 100

# Seed fixa para reprodução
python tests/heavy/fuzz/semantic/fuzz_semantic.py --seed 20260422 --iterations 500

# Timeout estendido
python tests/heavy/fuzz/semantic/fuzz_semantic.py --timeout 30.0
```

### Output

- **Crashes:** Salvos em `tests/heavy/fuzz/semantic/crashes/`
- **Hangs:** Salvos em `tests/heavy/fuzz/semantic/hangs/`
- **Report:** JSON + Markdown em `tests/heavy/reports/`

---

## 🚀 Execução

### Runner Principal

```bash
# Todos os heavy tests
python tests/heavy/run_heavy_tests.py

# Apenas security tests
python tests/heavy/run_heavy_tests.py --suite security

# Apenas crash tests
python tests/heavy/run_heavy_tests.py --suite crash

# Stress tests com timeout estendido
python tests/heavy/run_heavy_tests.py --suite stress --timeout 300

# Performance benchmarks
python tests/heavy/run_heavy_tests.py --suite performance
```

### Fuzzer

```bash
# Campanha padrão (100 iterações)
python tests/heavy/fuzz/semantic/fuzz_semantic.py

# Campanha estendida (1000 iterações)
python tests/heavy/fuzz/semantic/fuzz_semantic.py --iterations 1000 --timeout 15.0
```

---

## 📈 Critérios de Sucesso

| Categoria | Critério | Status |
|-----------|----------|--------|
| Security | Zero crashes/RCE/leaks | ⚠️ Aguardando execução |
| Crash | Zero crashes não documentados | ⚠️ Aguardando execução |
| Stress | Completa dentro do timeout | ⚠️ Aguardando execução |
| Performance | Map 10k inserts <5s | ⚠️ Aguardando execução |
| Correctness | Comportamento conforme spec | ⚠️ Aguardando execução |
| Fuzz | Zero crashes/hangs | ⚠️ Aguardando execução |

---

## 📊 Bugs Esperados (Baseado na Análise)

### Críticos (Devem Falhar)

| Bug | Teste | Expectativa |
|-----|-------|-------------|
| LEX-02: Hex bytes overflow | `buffer_overflow_hex` | ✗ Crash ou accept |
| SEM-01: used_params[128] | `buffer_overflow_params` | ✗ Crash ou accept |
| RUN-05: Command injection | `command_injection` | ✗ RCE possível |
| RUN-01: RC race | `rc_race_condition` | ⚠️ Pode passar (single-thread) |

### Altos (Podem Falhar)

| Bug | Teste | Expectativa |
|-----|-------|-------------|
| RUN-10: UTF-8 inválido | `utf8_invalid` | ✗ Crash ou UB |
| SEM-03: Integer overflow | `integer_overflow` | ⚠️ Pode passar (diagnóstico) |
| STD-03: Path traversal | `path_traversal` | ✗ Leitura bem-sucedida |

### Médios (Devem Passar)

| Bug | Teste | Expectativa |
|-----|-------|-------------|
| FUNC-01: Qualified names | `qualified_names` | ✓ Exit 42 |
| FUNC-03: Generic constraints | `generic_constraints` | ✓ Check fail |

---

## 🔍 Próximos Passos

1. **Executar testes** contra a codebase atual
2. **Coletar resultados** e identificar bugs reais
3. **Reportar bugs** encontrados com reprodução
4. **Expandir fuzzing** para 10k+ iterações
5. **Adicionar testes** para bugs descobertos

---

## 📝 Notas de Implementação

- **Timeouts:** Variam por categoria (30s padrão, 120s stress)
- **Isolamento:** Cada teste cria diretório temporário próprio
- **Reports:** JSON detalhado + Markdown summary
- **Reprodutibilidade:** Seeds salvas nos reports de crash/hang
- **LSP Errors:** Ignorados (são do código C, não dos testes Python)

---

*Heavy Tests implementados com base na análise profunda da codebase Zenith v0.3.0-alpha.1.*
