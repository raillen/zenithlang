# Behavior Tests

Projetos Zenith usados para validar comportamento observavel do MVP novo.

## Projetos atuais

- `simple_app/`: projeto valido. O entrypoint retorna `40 + 2`, logo o executavel deve sair com codigo `42`.
- `error_type_mismatch/`: projeto invalido. Deve falhar em semantica com span no arquivo `.zt`.
- `error_syntax/`: projeto invalido. Deve falhar no parser com span no arquivo `.zt`.

## Golden C

`simple_app/golden/simple-app.c` e o golden do C gerado pelo driver para o projeto `simple_app`.

O teste de conformance normaliza `CRLF` e `LF` antes de comparar, para manter o golden legivel e evitar ruido de plataforma.

## Build artifacts

Pastas `build/` dentro dos projetos de behavior sao saida de teste e ficam ignoradas por `.gitignore`.

- `control_flow_while/`: projeto valido. Exercita `while` no pipeline real at?? o execut??vel.
- `control_flow_repeat/`: projeto valido. Exercita `repeat N times` no pipeline real at?? o execut??vel.
- `control_flow_match/`: projeto valido. Exercita `match case/default` no pipeline real at?? o execut??vel.
- `control_flow_break_continue/`: projeto valido. Exercita `break` e `continue` dentro de `while`.

- `functions_calls/`: projeto valido. Exercita chamadas diretas, recursao simples e retornos `bool`/`float`/`text`/`void`/`int`.

- `functions_named_args/`: projeto valido. Exercita parametros nomeados em ordem declarada.

- `functions_defaults/`: projeto valido. Exercita valores padrao de parametros.

- `functions_main_signature_error/`: projeto invalido. Deve falhar na validacao do entrypoint `main`.

- `functions_invalid_call_error/`: projeto invalido. Deve falhar em semantica por argumento ausente com span.

- `structs_constructor/`: projeto valido. Exercita construcao de `struct` no pipeline real.

- `structs_field_defaults/`: projeto valido. Exercita defaults de campo em construtor de `struct`.

- `structs_field_read/`: projeto valido. Exercita leitura de campo de `struct`.

- `structs_field_update/`: projeto valido. Exercita atribuicao em campo de `struct`.

- `methods_inherent/`: projeto valido. Exercita metodo inerente via `apply Type`.

- `methods_mutating/`: projeto valido. Exercita metodo mutante com `mut func`.

- `methods_trait_apply/`: projeto valido. Exercita metodo de trait via `apply Trait to Type`.

- `list_basic/`: projeto valido. Exercita literal, indexacao 0-based e atualizacao de `list<int>`.
- `list_text_basic/`: projeto valido. Exercita literal, indexacao 0-based e atualizacao de `list<text>`.
- `map_basic/`: projeto valido. Exercita literal, indexacao por chave e atualizacao de `map<text,text>`.
- `optional_result_basic/`: projeto valido. Exercita `none`, `success` e `error`.

- `multifile_import_alias/`: projeto valido. Exercita varredura de `source.root` e chamada qualificada via import alias.
- `multifile_missing_import/`: projeto invalido. Deve falhar quando um import nao existe em `source.root`.
- `multifile_namespace_mismatch/`: projeto invalido. Deve falhar quando `namespace` nao corresponde ao caminho do arquivo.
- `multifile_duplicate_symbol/`: projeto invalido. Deve falhar quando dois arquivos geram o mesmo simbolo efetivo no programa agregado.
- `monomorphization_limit_error/`: projeto invalido. Deve falhar no gate de monomorfizacao quando `build.monomorphization_limit` ficar abaixo das instancias genericas reais.
- `where_contracts_ok/`: projeto valido. Exercita contratos `where` em parametro, construcao de `struct` e atribuicao de campo.
- `where_contract_param_error/`: projeto invalido em runtime. Deve falhar com `error[runtime.contract]` por violacao de contrato em parametro.
- `where_contract_construct_error/`: projeto invalido em runtime. Deve falhar com `error[runtime.contract]` por violacao de contrato em construcao de `struct`.
- `where_contract_field_assign_error/`: projeto invalido em runtime. Deve falhar com `error[runtime.contract]` por violacao de contrato em atribuicao de campo.

- `enum_match/`: fixture de comportamento para enum com payload + match com binding de payload (check semantico OK; build E2E bloqueado pelo stub de lowering HIR->ZIR no source atual).
- `enum_match_non_exhaustive_error/`: fixture invalida para diagnostico de match nao exaustivo em enum conhecido.