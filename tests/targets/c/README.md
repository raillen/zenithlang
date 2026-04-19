# Target C Tests

Golden tests e testes de integracao do backend C.

Suites atuais:

- \`tests/targets/c/test_legalization.c\`
- \`tests/targets/c/test_emitter.c\`

Comando base de compilacao da suite de legalization:

- \`gcc -std=c11 -Wall -Wextra -pedantic -I. compiler/zir/model.c compiler/targets/c/legalization.c tests/targets/c/test_legalization.c -o .ztc-tmp/tests/targets/c/test_legalization.exe\`

Comando base de compilacao da suite do emitter:

- \`gcc -std=c11 -Wall -Wextra -pedantic -I. compiler/zir/model.c compiler/zir/verifier.c compiler/targets/c/legalization.c compiler/targets/c/emitter.c tests/targets/c/test_emitter.c -o .ztc-tmp/tests/targets/c/test_emitter.exe\`

Validacao adicional esperada:

- compilar o C gerado em \`.ztc-tmp/tests/targets/c/generated_*.c\`
- para casos com runtime, linkar com \`runtime/c/zenith_rt.c\`
- executar pelo menos um caso com \`main\` real
- cobrir \`text\` e \`list<int>\` para \`index_seq\` / \`slice_seq\`
- cobrir \`make_list<int>\` e \`list_len\`


