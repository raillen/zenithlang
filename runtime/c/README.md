# Zenith Next Runtime C

Esta pasta concentra o runtime C do Zenith Next.

Arquivos ativos agora:

- `zenith_rt.h`: contrato inicial de ABI do runtime
- `zenith_rt.c`: implementacao inicial de RC, erros, texto, optionals, outcomes e listas especializadas

Subset implementado neste corte:

- tipos base `zt_int`, `zt_float`, `zt_bool`
- header comum de heap com RC
- `zt_retain` e `zt_release`
- `zt_runtime_error`, `zt_runtime_error_ex`, `zt_runtime_error_with_span`, `zt_runtime_report_error`, `zt_assert`, `zt_check`, `zt_panic`
- texto: `zt_text_from_utf8`, `zt_text_concat`, `zt_text_index`, `zt_text_slice`, `zt_text_eq`, `zt_text_len`
- lista especializada: `zt_list_i64_new`, `zt_list_i64_from_array`, `zt_list_i64_push`, `zt_list_i64_get`, `zt_list_i64_set`, `zt_list_i64_len`, `zt_list_i64_slice`
- lista especializada: `zt_list_text_new`, `zt_list_text_from_array`, `zt_list_text_push`, `zt_list_text_get`, `zt_list_text_set`, `zt_list_text_len`, `zt_list_text_slice`
- mapa especializado: `zt_map_text_text_new`, `zt_map_text_text_from_arrays`, `zt_map_text_text_set`, `zt_map_text_text_get`, `zt_map_text_text_len`
- `Optional<int>`: `zt_optional_i64_present`, `zt_optional_i64_empty`, `zt_optional_i64_is_present`, `zt_optional_i64_coalesce`
- `Optional<text>`: `zt_optional_text_present`, `zt_optional_text_empty`, `zt_optional_text_is_present`, `zt_optional_text_coalesce`
- `Optional<list<int>>`: `zt_optional_list_i64_present`, `zt_optional_list_i64_empty`, `zt_optional_list_i64_is_present`, `zt_optional_list_i64_coalesce`
- `Optional<list<text>>`: `zt_optional_list_text_present`, `zt_optional_list_text_empty`, `zt_optional_list_text_is_present`, `zt_optional_list_text_coalesce`
- `Optional<map<text,text>>`: `zt_optional_map_text_text_present`, `zt_optional_map_text_text_empty`, `zt_optional_map_text_text_is_present`, `zt_optional_map_text_text_coalesce`
- `Outcome<int,text>`: `zt_outcome_i64_text_success`, `zt_outcome_i64_text_failure`, `zt_outcome_i64_text_is_success`, `zt_outcome_i64_text_value`, `zt_outcome_i64_text_propagate`
- `Outcome<void,text>`: `zt_outcome_void_text_success`, `zt_outcome_void_text_failure`, `zt_outcome_void_text_is_success`, `zt_outcome_void_text_propagate`
- `Outcome<text,text>`: `zt_outcome_text_text_success`, `zt_outcome_text_text_failure`, `zt_outcome_text_text_is_success`, `zt_outcome_text_text_value`, `zt_outcome_text_text_propagate`
- `Outcome<list<int>,text>`: `zt_outcome_list_i64_text_success`, `zt_outcome_list_i64_text_failure`, `zt_outcome_list_i64_text_is_success`, `zt_outcome_list_i64_text_value`, `zt_outcome_list_i64_text_propagate`
- `Outcome<list<text>,text>`: `zt_outcome_list_text_text_success`, `zt_outcome_list_text_text_failure`, `zt_outcome_list_text_text_is_success`, `zt_outcome_list_text_text_value`, `zt_outcome_list_text_text_propagate`
- `Outcome<map<text,text>,text>`: `zt_outcome_map_text_text_success`, `zt_outcome_map_text_text_failure`, `zt_outcome_map_text_text_failure_message`, `zt_outcome_map_text_text_is_success`, `zt_outcome_map_text_text_value`, `zt_outcome_map_text_text_propagate`

Compatibilidade semantica atual:

- texto segue indices publicos 0-based
- `texto[i]` materializa novo `text` owned
- `slice` usa fim inclusivo
- `slice(..., -1)` significa ate o fim
- `list<int>` usa container owned com indices publicos 0-based
- `list<text>` retem elementos internamente e devolve `text` owned em `index_seq`
- `map<text,text>` usa container owned com busca linear no MVP, `map[key]` devolve `text` owned e `map_set` retem chave e valor internamente
- `Optional<text>` e `Optional<list<int>>` sao heap-managed para simplificar ownership no backend C
- `Outcome<int,text>`, `Outcome<void,text>`, `Outcome<text,text>`, `Outcome<list<int>,text>`, `Outcome<list<text>,text>` e `Outcome<map<text,text>,text>` sao heap-managed para simplificar ownership no backend C
- `zt_runtime_error_info` guarda kind, mensagem, codigo opcional e span opcional para diagnostico estruturado
- a boundary host minima do runtime C expoe `zt_host_read_file`, `zt_host_write_stdout` e `zt_host_write_stderr`, com override por `zt_host_set_api`

Estado atual do runtime:

- a ABI C cobre toda a superficie heap-managed que o compilador/target C expoe hoje
- erros runtime agora podem carregar span e codigo opcionais
- a boundary host minima ja esta presente para arquivo e streams
- novas specializations futuras passam a ser expansao de superficie da linguagem, nao gap do runtime atual
