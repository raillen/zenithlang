# M9 Conformance Tests

Testes end-to-end que provam o cutover do MVP para o pipeline novo.

O harness `test_m9.c` chama o driver real `compiler/driver/zt-next.exe` e valida:

- `project-info` resolve o projeto e o entrypoint.
- `check` (alias canonico) valida o pipeline de verificacao.
- `doc check` (forma canonica) valida ZDoc.
- `fmt --check` (gate bootstrap atual) esta disponivel.
- `test` (alias bootstrap atual) esta disponivel.
- `--ci` ativa renderer curto de diagnostics e saida previsivel para pipeline.
- `emit-c` gera C igual ao golden.
- `build` compila o C gerado.
- os harnesses usam sandbox temporario em `.ztc-tmp/.../sandbox` e forcam `output = "build_tmp"` para evitar lock de executavel no Windows.
- o executavel de `simple_app` retorna codigo `42`.
- erros sintaticos preservam span.
- erros semanticos preservam span.

Antes de rodar este harness, compile o driver.

## M16

`test_m16.c` e o harness final do corte frontend/backend atual. Ele roda todos os behavior projects validos, valida projetos invalidos por fragmentos golden de diagnostico e confere golden C/fragments representativos.

