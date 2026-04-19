# Runtime C Tests

Testes isolados do runtime C: RC, texto, bytes, listas, Optional, Outcome e erros.

Suite atual:

- `tests/runtime/c/test_runtime.c`

Comando base de compilacao do runtime test:

- `gcc -std=c11 -Wall -Wextra -pedantic -I. runtime/c/zenith_rt.c tests/runtime/c/test_runtime.c -o .ztc-tmp/tests/runtime/c/test_runtime.exe`

