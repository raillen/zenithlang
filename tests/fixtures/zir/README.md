# Zenith Next ZIR Fixtures

Esta pasta guarda fixtures pequenas para o schema textual da ZIR.

Primeiros casos ativos:

- `add.zir`
- `if_else.zir`
- `struct_init.zir`
- `index_seq.zir`
- `slice_seq.zir`
- `list_len.zir`
- `make_list.zir`
- `map_text_text.zir`
- `optional_present.zir`
- `optional_text.zir`
- `optional_list_int.zir`
- `outcome_try_propagate.zir`
- `outcome_void_try_propagate.zir`
- `outcome_text_try_propagate.zir`
- `coalesce.zir`
- `extern_c_call.zir`
- `invalid_any.zir`
- `invalid_target_leak.zir`

## Convencao de fixtures invalidas

Arquivos invalidos usam o padrao:

- `<nome>.zir`: entrada invalida
- `<nome>.error.txt`: erro estrutural esperado do verifier
