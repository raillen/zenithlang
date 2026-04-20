# Runtime C Tests

Testes isolados do runtime C: RC, texto, bytes, listas, Optional, Outcome, erros e wrappers compartilhados.

Suite atual:

- `tests/runtime/c/test_runtime.c`
- `tests/runtime/c/test_shared_text.c`

Comandos uteis de compilacao do runtime test:

- `gcc -std=c11 -Wall -Wextra -pedantic -I. runtime/c/zenith_rt.c tests/runtime/c/test_runtime.c -o .ztc-tmp/tests/runtime/c/test_runtime.exe`
- `gcc -std=c11 -Wall -Wextra -pedantic -I. runtime/c/zenith_rt.c tests/runtime/c/test_shared_text.c -lws2_32 -o .ztc-tmp/tests/runtime/c/test_shared_text.exe`

