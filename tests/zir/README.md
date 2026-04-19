# ZIR Tests

Suite do printer, verifier e lowering HIR -> ZIR.

Regra atual:

- os testes estruturais do compilador Next sao implementados em C;
- fixtures textuais continuam em \`.zir\`;
- \`.zt\` fica reservado para programas Zenith de entrada, nao para codigo do compilador.

Comando base de compilacao do printer test:

- \`gcc -std=c11 -Wall -Wextra -pedantic -I. compiler/zir/model.c compiler/zir/printer.c tests/zir/test_printer.c -o .ztc-tmp/tests/zir/test_printer.exe\`

Comando base de compilacao do verifier test:

- \`gcc -std=c11 -Wall -Wextra -pedantic -I. compiler/zir/model.c compiler/zir/verifier.c tests/zir/test_verifier.c -o .ztc-tmp/tests/zir/test_verifier.exe\`

