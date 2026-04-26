# Emitter Memory Profile

Gerado em: `2026-04-25T20:50:14.970229+00:00`

Escopo:
- mede o pico de working set do processo `zt.exe` no caminho `emit-c`;
- compara modo buffered (spill alto) com modo streamed/spill (spill baixo);
- valida equivalencia por `sha256` do C gerado.

Projeto sintetico: `.ztc-tmp/emitter-memory-profile/synthetic_app/zenith.ztproj`
Funcoes geradas: `7000`

| Modo | Spill threshold | Saida | Tempo | Pico de memoria | SHA256 |
|---|---:|---:|---:|---:|---|
| `buffered` | `268435456` | `2852832` bytes | `1391` ms | `68.34` MiB | `be90c9ae84e01b41` |
| `streamed` | `512` | `2852832` bytes | `1375` ms | `68.31` MiB | `be90c9ae84e01b41` |

Delta de pico (`streamed - buffered`): `-0.03` MiB
Delta de tempo (`streamed - buffered`): `-16` ms

Leitura:
- este teste nao falha por um threshold arbitrario de memoria;
- ele falha se o emit-c quebrar, se a medicao quebrar ou se a saida mudar;
- o snapshot serve para acompanhar tendencia real no `nightly/stress`.

