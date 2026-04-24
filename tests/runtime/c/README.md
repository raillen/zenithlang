# Runtime C Tests

Testes isolados do runtime C: RC, texto, bytes, listas, Optional, Outcome, erros e wrappers compartilhados.

Suite atual:

- `tests/runtime/c/test_runtime.c`
- `tests/runtime/c/test_net_error_kind.c`
- `tests/runtime/c/test_process_run.c`
- `tests/runtime/c/test_arithmetic_overflow.c`
- `tests/runtime/c/test_outcome_propagate.c`
- `tests/runtime/c/test_host_fs_guardrails.c`
- `tests/runtime/c/test_text_utf8_slice.c`
- `tests/runtime/c/test_text_utf8_guardrails.c`
- `tests/runtime/c/test_map_hash_table.c`
- `tests/runtime/c/test_thread_boundary_copy.c`
- `tests/runtime/c/test_runtime_error_tls.c`
- `tests/runtime/c/test_shared_text.c`

Comandos uteis de compilacao do runtime test:

- `gcc -std=c11 -Wall -Wextra -pedantic -I. runtime/c/zenith_rt.c tests/runtime/c/test_runtime.c -o .ztc-tmp/tests/runtime/c/test_runtime.exe`
- `gcc -std=c11 -Wall -Wextra -pedantic -I. runtime/c/zenith_rt.c tests/runtime/c/test_net_error_kind.c -o .ztc-tmp/tests/runtime/c/test_net_error_kind.exe -lws2_32`
- `gcc -std=c11 -Wall -Wextra -pedantic -I. runtime/c/zenith_rt.c tests/runtime/c/test_process_run.c -o .ztc-tmp/tests/runtime/c/test_process_run.exe -lws2_32`
- `gcc -std=c11 -Wall -Wextra -pedantic -I. runtime/c/zenith_rt.c tests/runtime/c/test_arithmetic_overflow.c -o .ztc-tmp/tests/runtime/c/test_arithmetic_overflow.exe -lws2_32`
- `gcc -std=c11 -Wall -Wextra -pedantic -I. runtime/c/zenith_rt.c tests/runtime/c/test_outcome_propagate.c -o .ztc-tmp/tests/runtime/c/test_outcome_propagate.exe -lws2_32`
- `gcc -std=c11 -Wall -Wextra -pedantic -I. runtime/c/zenith_rt.c tests/runtime/c/test_host_fs_guardrails.c -o .ztc-tmp/tests/runtime/c/test_host_fs_guardrails.exe -lws2_32`
- `gcc -std=c11 -Wall -Wextra -pedantic -I. runtime/c/zenith_rt.c tests/runtime/c/test_text_utf8_slice.c -o .ztc-tmp/tests/runtime/c/test_text_utf8_slice.exe -lws2_32`
- `gcc -std=c11 -Wall -Wextra -pedantic -I. runtime/c/zenith_rt.c tests/runtime/c/test_text_utf8_guardrails.c -o .ztc-tmp/tests/runtime/c/test_text_utf8_guardrails.exe -lws2_32`
- `gcc -std=c11 -Wall -Wextra -pedantic -I. runtime/c/zenith_rt.c tests/runtime/c/test_map_hash_table.c -o .ztc-tmp/tests/runtime/c/test_map_hash_table.exe -lws2_32`
- `gcc -std=c11 -Wall -Wextra -pedantic -I. runtime/c/zenith_rt.c tests/runtime/c/test_thread_boundary_copy.c -o .ztc-tmp/tests/runtime/c/test_thread_boundary_copy.exe -lws2_32`
- `gcc -std=c11 -Wall -Wextra -pedantic -I. runtime/c/zenith_rt.c tests/runtime/c/test_runtime_error_tls.c -o .ztc-tmp/tests/runtime/c/test_runtime_error_tls.exe -lws2_32`
- `gcc -std=c11 -Wall -Wextra -pedantic -I. runtime/c/zenith_rt.c tests/runtime/c/test_shared_text.c -lws2_32 -o .ztc-tmp/tests/runtime/c/test_shared_text.exe`
