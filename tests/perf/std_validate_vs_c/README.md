# std.validate vs C benchmark

Compara 3 caminhos para a mesma regra `value between [min, max]`:

- `std.validate.between` (Zenith-first)
- `c.zt_validate_between_i64` (helper em C)
- expressao inline (`>=` e `<=`)

## Rodar

```powershell
.\zt.exe run tests\perf\std_validate_vs_c
```

## Saida

A saida usa hexadecimal para facilitar no estado atual da stdlib (`std.format.hex`).

Exemplo:

```text
std.validate.between ms=0x34 hits=0x2a3f91
c.zt_validate_between_i64 ms=0x31 hits=0x2a3f91
inline >= <= ms=0x2f hits=0x2a3f91
```

Leitura rapida:
- Se os `hits` forem iguais, os 3 caminhos mantiveram a mesma semantica.
- Compare `ms` entre os casos na mesma maquina.
- Rode 3-5 vezes e use a mediana para reduzir ruido.
