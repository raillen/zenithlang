# Emitter Memory Profile

Gerado em: `2026-04-23T16:07:33.696405+00:00`

Escopo:
- mede o pico de working set do processo `zt.exe` no caminho `emit-c`;
- compara modo buffered (spill alto) com modo streamed/spill (spill baixo);
- valida equivalencia por `sha256` do C gerado.

Projeto sintetico: `.ztc-tmp/emitter-memory-profile/synthetic_app/zenith.ztproj`
Funcoes geradas: `7000`

| Modo | Spill threshold | Saida | Tempo | Pico de memoria | SHA256 |
|---|---:|---:|---:|---:|---|
| `buffered` | `268435456` | `2075669` bytes | `2046` ms | `63.49` MiB | `861f5c6fe938e32d` |
| `streamed` | `512` | `2075669` bytes | `2296` ms | `63.34` MiB | `861f5c6fe938e32d` |

Delta de pico (`streamed - buffered`): `-0.15` MiB
Delta de tempo (`streamed - buffered`): `250` ms

Leitura:
- este teste nao falha por um threshold arbitrario de memoria;
- ele falha se o emit-c quebrar, se a medicao quebrar ou se a saida mudar;
- o snapshot serve para acompanhar tendencia real no `nightly/stress`.

