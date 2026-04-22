# Heavy Tests — Zenith Language

Testes avançados para segurança, performance, stress e corretude da linguagem Zenith.

## Estrutura

```
tests/heavy/
├── security/           # Security vulnerability tests
│   ├── buffer_overflow/    # Buffer overflow exploits
│   ├── command_injection/  # Command injection (RCE)
│   ├── path_traversal/     # Directory traversal
│   └── race_conditions/    # Thread safety / data races
├── crash/              # Crash reproduction tests
│   ├── utf8_invalid/       # Invalid UTF-8 sequences
│   └── integer_overflow/   # Integer overflow UB
├── fuzz/               # Fuzzing tests
│   └── semantic/           # Semantic layer fuzzing
├── stress/             # Stress tests
│   ├── nesting/            # Deep nesting stress
│   └── loc/                # Large file stress (100k+ LOC)
├── correctness/        # Functional correctness
│   └── generics/           # Generic constraint validation
└── performance/        # Performance benchmarks
    └── maps/               # Map O(n) performance
```

## Execução

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

## Report

Os resultados são gerados em `tests/heavy/reports/`:
- `heavy-tests-YYYYMMDD-HHMMSS.json` — Resultados detalhados
- `heavy-tests-summary.md` — Resumo executivo

## Critérios de Falha

| Categoria | Critério |
|-----------|----------|
| Security | Qualquer crash, RCE, ou leak é **FAIL** |
| Crash | Crash não documentado é **FAIL** |
| Stress | Timeout ou OOM é **FAIL** |
| Performance | Degradação > 2x é **WARN** |
| Correctness | Comportamento incorreto é **FAIL** |
