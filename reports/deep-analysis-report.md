# Relatório Operacional de Pendências — Zenith Lang v2

**Data:** 2026-04-23  
**Versão analisada:** 0.3.0-alpha.1  
**Escopo:** compilador, runtime C, stdlib, tooling, specs e testes  
**Objetivo deste documento:** manter somente pendências reais ou de alta confiança, com plano de implementação, roadmap de bugfix e checklist executável.

---

## 1. Critério deste relatório

Este documento mantém apenas:

- bugs confirmados ou de alta confiança no código atual;
- lacunas concretas de runtime, stdlib, tooling e testes;
- warnings arquiteturais que precisam de decisão explícita;
- drift documental que hoje pode induzir correções erradas, releases confusos ou auditorias ruins.

Este documento remove, de propósito:

- achados já refutados pelo código atual;
- hipóteses sem evidência direta suficiente;
- duplicações entre segurança, corretude, performance e documentação;
- gates antigos reabertos sem revalidação.

Itens refutados e portanto removidos da versão operacional:

- overflow em `zt_parser_normalize_hex_bytes` como bug ativo de buffer overflow;
- crash/UB por `used_params[128]` como descrito no relatório anterior;
- leak operacional em `zt_parser_strdup` sem uso comprovado no repositório atual.

---

## 2. Resumo executivo

| Prioridade | Quantidade | Blocos principais |
|---|---:|---|
| P0 | 0 | sem bloqueios P0 restantes neste recorte |
| P1 | 1 | fronteira residual de thread-safety/ARC |
| P2 | 2 | emitter streaming e alinhamento documental residual |
| P3 | 1 | hardening sistemático de testes |

### Ordem recomendada

1. Consolidar a política residual de `single-isolate`/ARC antes de qualquer passo concreto em paralelismo.
2. Tratar emitter streaming como trilha de escalabilidade, não como bugfix imediato.
3. Fortalecer a esteira de testes agora que os guardas de profundidade, UTF-8 interno e `queue/stack` públicos já estão fechados neste recorte.

---

## 3. Backlog operacional

### BF-01 — Command injection em `std.os.process.run`
- Prioridade: `P0`
- Estado: `Corrigido no working tree (2026-04-22)`
- Área: `Runtime / Segurança`
- Evidência: `zt_host_default_process_run` monta uma string com programa + argumentos e executa `system(command)`.
- Risco: qualquer argumento com metacaracteres de shell pode mudar o comando executado.
- Avanço no working tree: o caminho user-facing saiu de `system()` e passou a executar por `argv` real, sem shell; a suíte C cobre exit code, bloqueio de injection e `cwd`, e os behaviors `std_os_process_basic` / `std_os_process_capture_basic` passam com `ZENITH_HOME` local.
- Implementação:
1. Substituir o caminho user-facing baseado em `system()` por uma API sem shell.
2. No Windows, usar `CreateProcessW` com montagem segura de linha de comando e escaping correto.
3. Em POSIX, usar `fork + execve` ou `posix_spawn` com vetor `argv` separado.
4. Manter `cwd` como responsabilidade do host layer, sem concatenar comando com `cd`.
5. Preservar retorno tipado de `ExitStatus` e mapeamento de erro para `std.os.process.Error`.
6. Proibir qualquer fallback silencioso para `system()` em builds normais.
- Testes necessários:
1. Args contendo `;`, `&&`, `|`, `$()`, aspas, backslashes e espaços.
2. Programa com caminho contendo espaços.
3. `cwd` válido, inválido e ausente.
4. Casos com código de saída zero e não zero.
5. Caso com binário inexistente e sem permissão.
- Critério de conclusão:
1. Nenhum fluxo user-facing de execução de processo chama `system()`.
2. Suíte de regressão de argumentos hostis aprovada.
3. Documentação de `std.os.process` atualizada com semântica de quoting e `cwd`.

### BF-02 — Overflow de tamanho em `zt_text_concat` e `zt_bytes_join`
- Prioridade: `P0`
- Estado: `Corrigido no working tree (2026-04-22)`
- Área: `Runtime / Segurança / Corretude`
- Evidência: somas de comprimentos são feitas antes da alocação sem guarda explícita.
- Risco: overflow de `size_t`, alocação menor que a necessária e escrita fora do buffer.
- Avanço no working tree: o runtime ganhou helpers de soma segura antes de alocação/cópia; `zt_text_concat` e `zt_bytes_join` falham com erro controlado, e a suíte C cobre os caminhos de overflow em subprocesso.
- Implementação:
1. Criar helpers internos `zt_size_add_checked` e `zt_size_add1_checked`.
2. Usar esses helpers antes de qualquer `malloc`, `calloc` ou `from_array/from_utf8` com tamanho derivado de soma.
3. Falhar com erro de runtime estável e mensagem específica de overflow de tamanho.
4. Revisar chamadas similares em concatenação de path, split/join e buffers temporários.
5. Centralizar padrão para evitar regressão em novos helpers.
- Testes necessários:
1. Testes unitários para overflow próximo de `SIZE_MAX` em texto e bytes.
2. Casos normais pequenos e médios para garantir sem regressão funcional.
3. Testes em plataformas 64-bit e, se possível, 32-bit.
- Critério de conclusão:
1. Nenhuma rota de concatenação faz soma de tamanho sem guarda.
2. Overflow vira erro controlado, não comportamento indefinido.

### BF-03 — Fronteira de thread-safety indefinida no runtime e em globals do driver
- Prioridade: `P1`
- Estado: `Parcialmente corrigido no working tree (2026-04-22)`
- Área: `Runtime / Driver / Arquitetura`
- Evidência: `zt_retain` e `zt_release` não são atômicos; `zt_last_error` e globals do driver não têm isolamento por contexto.
- Risco: se a base evoluir para paralelismo real, surgem double-free, UAF, vazamento de estado e resultados não determinísticos.
- Situação atual já documentada: o caminho padrão do runtime é `single-isolate` com ARC não atômico; fronteiras de concorrência usam `isolate/message-passing`; a referência canônica é `language/spec/runtime-model.md`.
- Avanço no working tree: o estado mutável de invocação do driver foi movido para `zt_driver_context`, incluindo `ci`, perfil, filtros, manifesto ativo, root do projeto e telemetry; o driver recompila com esse fluxo.
- Implementação:
1. Tomar uma decisão explícita: `single-thread only` no MVP ou runtime thread-safe mínimo.
2. Se a decisão for manter single-thread only, documentar isso em spec, README e comentários de API, e bloquear chamadas paralelas no roadmap oficial.
3. Se a decisão for suportar threads, introduzir atomics em RC, revisar `zt_last_error` para contexto por thread e remover globals de driver em favor de contexto explícito.
4. Passar `manifest`, `ci_mode`, `show_all_errors` e flags afins por estruturas de contexto.
5. Isolar futuras features de build paralelo atrás desse mesmo contexto.
- Testes necessários:
1. Stress test multi-thread de retain/release.
2. Execuções concorrentes do compilador com projetos diferentes.
3. Testes de isolamento de erro e flags entre compilações consecutivas.
- Critério de conclusão:
1. A política de thread-safety fica explícita no produto.
2. O código segue a política escolhida sem ambiguidades.
3. Não restam globals mutáveis relevantes sem dono/contexto definido.

### BF-04 — Scaffolding de ciclo RC perigoso e morto
- Prioridade: `P1`
- Estado: `Corrigido no working tree (2026-04-22)`
- Área: `Runtime / Higiene`
- Evidência: `zt_detect_cycles` altera o bit alto de `rc`; `zt_runtime_check_for_cycles` é stub; o bloco não participa do fluxo normal.
- Risco: código morto perigoso confunde auditoria e pode ser religado de forma insegura no futuro.
- Referência canônica de política: `language/spec/runtime-model.md`; `README.md`, `language/MVP_OUT_OF_SCOPE.md` e `language/surface-implementation-status.md` foram sincronizados para apontar o modelo atual.
- Implementação:
1. Remover `zt_detect_cycles`, `zt_runtime_check_for_cycles` e helpers scaffolding associados que não participam do runtime real.
2. Se a equipe quiser preservar a ideia, mover para documento de design ou branch experimental, não para o runtime principal.
3. Registrar política oficial de ciclos RC em documento de risco, com status e decisões.
- Testes necessários:
1. Build limpo do runtime após remoção.
2. Busca estática garantindo ausência de chamadas restantes.
- Critério de conclusão:
1. Não existe mais código morto de detecção de ciclos no runtime principal.
2. A política para ciclos está documentada em um único lugar.

### BF-05 — `text_index` e `text_slice` podem devolver UTF-8 inválido
- Prioridade: `P1`
- Estado: `Corrigido no working tree (2026-04-22)`
- Área: `Runtime / Semântica`
- Evidência: indexação e slice operam por byte e podem cortar no meio de sequência multi-byte.
- Risco: textos inválidos saem do runtime e quebram invariantes da linguagem.
- Referência canônica já aceita: `language/decisions/018-literals-text-indexing-and-slices.md` fixa `text` index/slice por code point; `language/decisions/059-stdlib-text.md` e `stdlib/zdoc/std/text.zdoc` repetem o mesmo modelo.
- Avanço no working tree: `zt_text_index`, `zt_text_slice` e `zt_text_len` foram alinhados ao modelo por code point; foi adicionada suíte C focada em caracteres de 2, 3 e 4 bytes e behavior project `text_utf8_index_slice`, além de cobertura UTF-8 em `std_validate_basic`.
- Implementação:
1. Definir na spec se `text` indexa por byte, scalar Unicode ou grapheme cluster.
2. Se a semântica desejada for Unicode-safe, implementar navegação por fronteira UTF-8 antes de indexar/slicear.
3. Se a semântica desejada for byte-level, renomear/documentar a API e proibir vender isso como `text` semanticamente seguro.
4. Adicionar erro de runtime dedicado quando o corte solicitado cair em fronteira inválida.
- Testes necessários:
1. Casos com ASCII simples.
2. Casos com caracteres de 2, 3 e 4 bytes.
3. Slice no início, meio e fim de sequência multi-byte.
4. Casos mistos com emoji e acentos.
- Critério de conclusão:
1. A semântica fica explícita na spec.
2. O runtime não devolve UTF-8 inválido como se fosse `text` válido.

### BF-06 — Leak em `zt_net_error_kind_index`
- Prioridade: `P1`
- Estado: `Corrigido no working tree (2026-04-22)`
- Área: `Runtime / Memória`
- Evidência: a função cria vários `zt_text_from_utf8_literal(...)` por chamada sem liberar.
- Risco: vazamento repetitivo em caminhos de erro de rede.
- Avanço no working tree: a classificação agora compara com literais C sem alocação temporária; foi adicionada suíte dedicada com casos conhecidos, desconhecidos, `NULL` e loop grande de repetição.
- Implementação:
1. Remover comparações baseadas em alocação temporária.
2. Comparar `error.code->data` diretamente com literais C quando seguro.
3. Alternativamente, mapear códigos de erro para enum interno antes do ponto de chamada.
4. Revisar outras funções utilitárias que usam `zt_text_from_utf8_literal` apenas para comparação.
- Testes necessários:
1. Repetir chamadas em loop grande e verificar estabilidade de memória.
2. Casos para cada erro conhecido e para erro desconhecido.
- Critério de conclusão:
1. Não há mais alocação temporária por consulta de tipo de erro.
2. Teste de repetição não mostra crescimento de memória.

### BF-07 — Queue/stack mutam sem respeitar COW
- Prioridade: `P1`
- Estado: `Corrigido no working tree (2026-04-23)`
- Área: `Stdlib / Coleções`
- Evidência original: `dequeue` e `pop` mutavam a estrutura diretamente, ao contrário do padrão `*_owned` já usado em outras coleções.
- Risco original: semântica inconsistente de ownership e isolamento após cópia/compartilhamento.
- Avanço no working tree: a superfície pública de `std.collections` agora devolve um resultado estruturado com `{colecao_atualizada, value}` para `queue_*_dequeue` e `stack_*_pop`; isso preserva COW sem depender do runtime C antigo. Foi adicionado behavior E2E cobrindo cópia compartilhada, casos vazios e tipos `int`/`text`.
- Observação residual: os helpers crus do runtime C continuam existindo como detalhe interno/compatibilidade, mas o contrato user-facing da linguagem ficou coerente.
- Implementação:
1. Definir contrato unificado de mutação para queue/stack.
2. Expor `dequeue/pop` como operações que devolvem coleção atualizada junto com o item removido.
3. Alinhar ZDoc, matriz e fixture de comportamento ao novo contrato.
4. Registrar a aresta restante do emitter: promoção implícita para `optional<T>` dentro de `make_struct` ainda precisou de helper tipado.
- Testes necessários:
1. Compartilhar queue/stack e mutar cada cópia.
2. Casos com tipos simples e `text`.
3. Casos vazios e com múltiplos elementos.
- Critério de conclusão:
1. Queue/stack seguem a mesma regra de ownership das demais coleções do runtime.
2. Há regressão E2E cobrindo cópia e mutação isolada.

### BF-08 — Validação de chamadas com alta aridade depende de array fixo e retorna cedo
- Prioridade: `P1`
- Estado: `Corrigido no working tree (2026-04-22)`
- Área: `Typechecker / Corretude`
- Evidência: `used_params[128]` foi protegido contra overflow, mas a proteção atual apenas retorna cedo e pode deixar o caso sem diagnóstico claro.
- Risco: chamadas com muitos parâmetros podem escapar da validação esperada ou produzir resultado silenciosamente incompleto.
- Avanço no working tree: `used_params[128]` foi trocado por vetor dinâmico baseado em `params.count`, com diagnóstico explícito em OOM; há suíte semântica focada para chamadas com 129 parâmetros.
- Implementação:
1. Trocar `used_params[128]` por bitmap/vetor dinâmico alocado pelo número real de parâmetros.
2. Em caso de OOM, emitir diagnóstico explícito e controlado.
3. Revisar pontos equivalentes em validação de campos/variantes para evitar caps escondidos.
4. Adicionar diagnóstico específico para aridade extrema, se a linguagem quiser impor limite formal.
- Testes necessários:
1. Função com 129, 256 e 1024 parâmetros em fixture sintética.
2. Mistura de posicionais e nomeados.
3. Casos duplicados, faltantes e fora de ordem em alta aridade.
- Critério de conclusão:
1. Não existe mais cap implícito de 128 parâmetros no checker.
2. Casos grandes são validados ou rejeitados com diagnóstico explícito.

### BF-09 — Portabilidade quebrada para MSVC por uso direto de `__builtin_*_overflow`
- Prioridade: `P1`
- Estado: `Corrigido no working tree (2026-04-22)`
- Área: `Runtime / Portabilidade`
- Evidência: builtins GNU são usados diretamente nas operações aritméticas de overflow.
- Risco: build falha no Windows com MSVC, apesar de o projeto mirar ambiente Windows.
- Avanço no working tree: a aritmética de overflow foi encapsulada com fallback portátil e validada em três caminhos: GCC/Clang com builtins, fallback forçado e `clang-cl` no caminho compatível com MSVC.
- Implementação:
1. Criar camada `zt_overflow.h/.c` ou helpers estáticos por compilador.
2. Em GCC/Clang, manter builtins existentes.
3. Em MSVC, usar intrinsics equivalentes ou fallback manual seguro.
4. Garantir mesma semântica de erro de runtime em todos os compiladores.
- Testes necessários:
1. Build com GCC/Clang.
2. Build com MSVC.
3. Casos positivos e overflow real para soma, subtração e multiplicação.
- Critério de conclusão:
1. O runtime compila em GCC/Clang/MSVC.
2. O comportamento observável em overflow é o mesmo em todos eles.

### BF-10 — `c_string_buffer` cresce sem streaming ou flush
- Prioridade: `P2`
- Estado: `Parcialmente corrigido no working tree (2026-04-23)`
- Área: `Backend C / Performance`
- Evidência: o emitter acumula tudo em memória antes de persistir resultado.
- Risco: pico de memória cresce com tamanho do módulo e limita projetos maiores.
- Avanço no working tree: o emitter agora pode fazer spill para `tmpfile()` acima de um limiar configurável (`ZT_EMITTER_SPILL_THRESHOLD_BYTES`) e o caminho de `build/run` passou a persistir via `c_emitter_write_file(...)` sem materializar o C inteiro em RAM; `hardening/concurrent_compilation` força o spill com limite baixo e segue verde em `nightly`.
- Residual aberto: `emit-c` para stdout e os testes que chamam `c_emitter_text(...)` ainda materializam tudo em memória no fim; ainda não existe relatório objetivo de pico de memória antes/depois.
- Implementação:
1. Introduzir writer abstrato com suporte a buffer em memória e stream para arquivo.
2. Manter modo atual para testes pequenos e modo streaming para `emit-c/build`.
3. Preservar mensagens de erro e rollback local com marks apenas onde realmente necessário.
4. Medir memória máxima antes e depois da mudança.
- Testes necessários:
1. Emitir módulo pequeno e comparar saída byte a byte com baseline atual.
2. Emitir módulo grande sintético e medir pico de memória.
3. Falhas de escrita em stream para validar propagação de erro.
- Critério de conclusão:
1. `emit-c/build` podem operar sem manter todo o módulo gerado em RAM.
2. Saída final permanece estável para módulos existentes.

### BF-11 — `map<K,V>` continua com busca linear no runtime
- Prioridade: `P2`
- Estado: `Corrigido no working tree (2026-04-23)`
- Área: `Runtime / Performance`
- Evidência original: operações seguiam caminho linear e a infraestrutura de hash não estava fechada como benefício real para o caminho genérico.
- Avanço no working tree: `map<text,text>` passou a manter índice hash com probing aberto no template do runtime, preservando ordem de iteração por arrays; a suíte `tests/runtime/c/test_map_hash_table.c` cobre volume, overwrite e COW; `tests/behavior/std_json_basic` continua passando sobre `map<text,text>`.
- Validação adicional: o emitter C foi sincronizado com a assinatura hash do runtime, `pr_gate` voltou a `134/134` e `nightly --no-perf` ficou verde com a trilha atual.
- Nota de fechamento: o recorte alpha atual expõe apenas `map<text,text>` como caminho genérico relevante. Publicação de benchmark formal por faixa de tamanho pode seguir como melhoria de hardening, mas o problema descrito neste item deixou de existir no runtime atual.
- Implementação:
1. Decidir se o MVP quer manter `map<K,V>` genérico com performance básica ou subir o nível de implementação.
2. Se mantiver o tipo como superfície principal, implementar índice/hash consistente para lookup/set/remove.
3. Se não, documentar claramente o custo atual e orientar uso de estruturas especializadas quando aplicável.
4. Adicionar benchmarks fixos para 1k, 10k e 100k entradas.
- Testes necessários:
1. Benchmarks de insert/get/remove.
2. Casos com colisão alta.
3. Casos de correção com sobrescrita e remoção.
- Critério de conclusão:
1. Complexidade real e documentação passam a bater.
2. O benchmark mostra ganho mensurável ou o contrato de performance fica explicitamente rebaixado.

### BF-12 — `zt_catalog_find_decl()` continua linear no checker
- Prioridade: `P2`
- Estado: `Corrigido no working tree (2026-04-23)`
- Área: `Typechecker / Performance`
- Evidência original: resolução de declarações no catálogo fazia varredura linear por nome.
- Avanço no working tree: `zt_module_catalog` agora constrói índice hash para declarações, `zt_catalog_find_decl()` consulta o índice primeiro e a suíte sintética `tests/semantic/test_catalog_lookup_scale.c` passa com `3504/3504`.
- Nota de fechamento: o lookup quente de `decl` saiu do caminho linear; o que ainda pode merecer trilha futura de performance são aliases de import e listas de `apply`, mas isso não é mais o problema descrito neste item.
- Implementação:
1. Introduzir índice `name -> decl` durante a montagem do catálogo, preservando as regras atuais de namespace e duplicidade.
2. Se o índice global for invasivo demais neste corte, adicionar memoização por consulta ou cache por módulo como passo intermediário.
3. Garantir que nomes ambíguos e conflitos continuem produzindo o mesmo diagnóstico observável.
4. Medir impacto em projetos sintéticos com 2k, 5k e 10k declarações.
- Testes necessários:
1. Fixture sintético com milhares de declarações e lookup repetido.
2. Casos com nomes duplicados/sombreados para garantir que o índice não mascara erro semântico.
3. Benchmark comparando antes/depois no `check`.
- Critério de conclusão:
1. Lookup de declaração deixa de ser O(n) no caminho quente principal.
2. Diagnósticos e resolução nominal permanecem estáveis.

### BF-13 — Parser e lowering ainda não têm limite estrutural explícito
- Prioridade: `P3`
- Estado: `Corrigido no working tree (2026-04-23)`
- Área: `Frontend / ZIR lowering / Robustez`
- Evidência original: parser e lowering continuavam recursivos sem depth guard formal para nesting extremo.
- Avanço no working tree: parser ganhou limite explícito de nesting com `ZT_DIAG_STRUCTURE_LIMIT_EXCEEDED`; o lowering passou a validar profundidade de HIR antes da emissão ZIR; as suítes focadas `tests/frontend/test_parser_depth_guard.c` e `tests/zir/test_lowering_depth_guard.c` passam.
- Nota de fechamento: o limite atual é guarda de robustez do alpha, não semântica da linguagem.
- Implementação:
1. Adicionar contador de profundidade no parser recursivo e no lowering de HIR/ZIR.
2. Definir limite inicial conservador para alpha e emitir diagnóstico estável quando o limite for ultrapassado.
3. Cobrir blocos, expressões, tipos genéricos e `match`/`if` profundamente aninhados.
4. Documentar o limite na spec/README como guarda de robustez, não como parte da semântica da linguagem.
- Testes necessários:
1. Fixtures com nesting profundo de `if`, `match`, listas/tipos e expressões.
2. Caso limítrofe que ainda deve passar.
3. Caso acima do limite que precisa falhar com diagnóstico e sem crash.
- Critério de conclusão:
1. Nenhuma entrada profundamente aninhada derruba parser/lowering por stack overflow silencioso.
2. O limite é testado e diagnosticado de forma estável.

### BF-14 — `zt_text_from_utf8()` ainda é construtor interno sem validação
- Prioridade: `P3`
- Estado: `Corrigido no working tree (2026-04-23)`
- Área: `Runtime / Invariantes de encoding`
- Evidência original: a leitura default de arquivo já validava UTF-8 e o host filesystem já aplicava guardrails de path, mas o construtor bruto `zt_text_from_utf8()` continuava aceitando bytes arbitrários.
- Avanço no working tree: `zt_text_from_utf8()` virou caminho validado, o construtor bruto foi isolado como helper interno `unchecked`, stdin passou a devolver erro controlado para UTF-8 inválido, e a suíte `tests/runtime/c/test_text_utf8_guardrails.c` cobre caminho válido + falha em subprocesso.
- Nota de fechamento: ainda existem pontos de auditoria futura para `char *` dinâmico vindo do host, mas o construtor público/host descrito neste item deixou de aceitar bytes arbitrários sem validação.
- Implementação:
1. Renomear o construtor atual para variante explicitamente `unchecked` interna, ou introduzir um construtor validado público e restringir o bruto ao runtime privado.
2. Auditar entradas de host (`stdin`, env, cwd, path) para decidir quais precisam retornar `result` em vez de falhar abruptamente.
3. Consolidar helper único de validação UTF-8 para todas as fronteiras externas.
4. Documentar claramente a diferença entre caminho validado e caminho interno bruto.
- Testes necessários:
1. Bytes com UTF-8 inválido em caminhos user-facing devem falhar com erro controlado.
2. Guardar regressão para overlong, surrogate e continuation inválida.
3. Verificar que helpers internos que já recebem dados validados continuam funcionando sem custo extra visível.
- Critério de conclusão:
1. Não existe mais API pública/host que consiga fabricar `text` inválido sem passar por validação.
2. O construtor bruto, se ainda existir, fica isolado e nomeado como operação insegura interna.

### DOC-01 — Taxonomia de diagnósticos runtime diverge da spec
- Prioridade: `P2`
- Estado: `Corrigido no working tree (2026-04-22)`
- Área: `Spec / Diagnostics / Tooling`
- Evidência: a spec exige códigos como `runtime.contract`, `runtime.bounds`, `runtime.map_key`, `runtime.utf8`, `runtime.divide_by_zero`, enquanto o catálogo e o runtime expõem nomes diferentes.
- Risco: ferramentas, LSP, docs e testes podem falar línguas diferentes sobre o mesmo erro.
- Avanço no working tree: `language/spec/diagnostics-model.md` e `language/spec/diagnostic-code-catalog.md` agora apontam para a mesma taxonomia alpha (`runtime.assert`, `runtime.check`, `runtime.contract`, `runtime.index`, `runtime.io`, `runtime.math`, `runtime.panic`, `runtime.platform`, `runtime.unwrap`), e `language/surface-implementation-status.md` deixou de tratar isso como risco aberto.
- Implementação:
1. Escolher a fonte normativa única: spec ou catálogo atual.
2. Se a spec vencer, alinhar renderer, runtime e fixtures aos códigos normativos.
3. Se o catálogo atual vencer, atualizar spec, decisions e reports antigos para os nomes reais.
4. Revisar mensagens de erro e fixtures `.contains.txt` afetadas.
- Testes necessários:
1. Golden tests de diagnósticos runtime.
2. Verificação de compatibilidade no renderer e no `zt verify`.
- Critério de conclusão:
1. Existe uma única taxonomia oficial.
2. Spec, runtime, catálogo e fixtures usam os mesmos nomes.

### DOC-02 — `MVP_OUT_OF_SCOPE`, status de superfície e decisões estão em conflito
- Prioridade: `P2`
- Estado: `Corrigido no working tree (2026-04-22)`
- Área: `Documentação / Governança`
- Evidência: multifile, `.ztproj` e ZDoc aparecem como post-MVP em um lugar e conformantes em outro.
- Risco: roadmap errado, auditoria errada e contribuições desalinhadas.
- Avanço no working tree: `language/MVP_OUT_OF_SCOPE.md` foi reposicionado como nota histórica + lista de itens ainda deferidos; `language/surface-implementation-status.md` e `README.md` agora apontam explicitamente quais documentos descrevem o corte alpha atual.
- Implementação:
1. Escolher o documento canônico para “estado do corte atual”.
2. Atualizar `language/MVP_OUT_OF_SCOPE.md` para refletir o corte real ou renomeá-lo para histórico.
3. Revisar `surface-implementation-status.md`, `conformance-matrix`, README e decisões relacionadas.
4. Criar regra editorial: documento normativo precisa trazer data, status e precedência.
- Testes necessários:
1. Revisão manual cruzada de links e referências.
2. Checklist documental no release process.
- Critério de conclusão:
1. Não existem mais conflitos abertos entre status do corte, MVP e superfície conformante.
2. O time consegue responder “isso é MVP ou não?” em um único documento.

### STDLIB-01 — `std.text` foi alinhado ao subset alpha seguro
- Prioridade: `P2`
- Estado: `Corrigido no working tree (2026-04-22)`
- Área: `Stdlib / Surface alignment`
- Evidência anterior: a decisão e o ZDoc vendiam uma primeira onda ampla, enquanto o runtime real só tinha primitives mínimas.
- Avanço no working tree: `std.text` deixou de declarar externs fictícios e hoje expõe apenas o subset implementado com segurança no alpha atual:
1. `trim`, `trim_start`, `trim_end`
2. `contains`, `starts_with`, `ends_with`, `has_prefix`, `has_suffix`
3. `has_whitespace`, `index_of`, `last_index_of`
4. `is_empty`, `is_digits`, `limit`
5. `to_utf8`, `from_utf8`
- Validação atual:
1. `tests/behavior/std_text_basic` passa com `exit 0`
2. `tests/behavior/std_bytes_utf8` continua cobrindo `to_utf8` / `from_utf8`
3. ZIR verifier ganhou regressão para falso positivo em `suffix`/`ffi`
- Nota importante do alpha:
1. `from_utf8(...)` retorna `result<text, text>` neste corte
2. `text.Error` tipado, casing, split/join, replace, padding, mask e truncate deixam de ser pendência de backend e passam a ser expansão futura de superfície
3. as duas arestas de backend que bloqueavam o crescimento imediato de `std.text` foram fechadas neste working tree:
   - cópia/propagação segura de `result<text,text>` com payload gerenciado
   - comparação direta entre retorno de chamada `text` e literal no emitter C
4. permanece uma aresta semântica separada: `case success(value)` e `case error(err)` ainda pedem um passe dedicado para permitir comparação direta do binding com literal sem passar por uma variável `text` explícita
- Critério de conclusão:
1. `std.text` não promete helpers sem implementação real
2. docs, behavior tests e módulo público batem com a superfície entregue

### STDLIB-02 — `std.fs` foi fechado no recorte alpha atual
- Prioridade: `P2`
- Estado: `Corrigido no working tree (2026-04-22)`
- Área: `Stdlib / Host API`
- Evidência anterior: `std.fs` declarava uma camada mais rica que a realmente entregue pelo runtime.
- Avanço no working tree:
1. o runtime ganhou wrappers portaveis para append, kind checks, create/remove, list, copy, move e timestamps
2. `std.fs` agora mapeia `core.Error` para `fs.Error`
3. `fs.Metadata` foi estabilizado com `size_bytes`, `modified_at_ms`, `created_at_ms`, `is_file` e `is_dir`
4. o nome publico de listagem neste corte ficou `list_dir(...)`
- Validação atual:
1. `tests/behavior/std_fs_basic` passa
2. `tests/behavior/std_fs_ops_basic` passa
3. `tests/runtime/c/test_host_fs_guardrails.c` passa cobrindo UTF-8 inválido, `ZENITH_HOST_FS_ROOT` e caminho com NUL
- Nota importante do alpha:
1. timestamps ficam em Unix milliseconds (`int` / `optional<int>`) neste corte
2. a decisao e o ZDoc precisam refletir isso, em vez de prometer `time.Instant`
- Critério de conclusão:
1. `std.fs` deixou de anunciar backend ausente no recorte alpha atual
2. fixtures reais cobrem create/list/metadata/remove/copy/move

### STDLIB-03 — lacunas restantes em `std.os` e `std.os.process`
- Prioridade: `P2`
- Estado: `Corrigido no working tree (2026-04-22)`
- Área: `Stdlib / Runtime / Host API`
- Funções pendentes:
1. `zt_host_os_args`
2. `zt_host_process_run_capture`
- Risco: `std.os` e `std.os.process` parecem mais completos do que realmente estão.
- Avanço no working tree: o runtime passou a capturar `argv` inicial para `os.args()`, ganhou `process.run_capture()` com stdout/stderr separados e exit code, e o fechamento foi validado com suíte C mais os behaviors `std_os_args_basic`, `std_os_basic`, `std_os_process_basic` e `std_os_process_capture_basic`.
- Implementação:
1. Introduzir armazenamento inicial de `argv` no host runtime para `zt_host_os_args`.
2. Implementar `run_capture` com captura separada de stdout e stderr, sem shell.
3. Reusar a mesma infraestrutura segura criada em `BF-01`.
4. Mapear corretamente encoding, falha de spawn e timeout futuro, se existir.
- Testes necessários:
1. `os.args()` com zero, um e múltiplos argumentos.
2. `run_capture()` retornando stdout, stderr e exit code.
3. Processo inexistente e processo com falha.
- Critério de conclusão:
1. As duas funções passam a existir no runtime com comportamento documentado.
2. `std.os` e `std.os.process` fecham a superfície declarada do corte atual.

### TEST-01 — backlog focal de hardening ainda aberto
- Prioridade: `P3`
- Estado: `Parcialmente corrigido no working tree (2026-04-23)`
- Área: `Qualidade / Testes`
- Lacunas a fechar:
1. integrar mais subconjuntos estáveis de `tests/heavy` ao gate oficial além da fuzz semântica já promovida;
2. publicar alvo de coverage com relatório simples por arquivo ou por módulo;
3. decidir se vale promover o harness concorrente também para `pr_gate` ou mantê-lo em `nightly/stress`;
4. continuar ampliando a matriz do formatter quando novas superfícies entrarem no alpha.
- Situação atual já presente no repositório:
1. hardening gates já rodam em `pr_gate`/`nightly`/`stress` via `tests/hardening/*.py`;
2. existe fuzzing leve em `tests/fuzz/` para lexer/parser;
3. `tests/heavy/fuzz/semantic/fuzz_semantic.py` agora roda em `nightly/stress` como `stress/fuzz_semantic`, com seed fixa e iterações curtas/longas por suite;
4. `tests/hardening/test_concurrent_compilation.py` agora valida builds nativos paralelos em cache frio/quente dentro de `nightly/stress`;
5. `tests/formatter/run_formatter_golden.py` saiu de `case_all` isolado para 9 casos cobrindo imports, structs, match, generics, comments, triple-quoted text, trailing comma em lista e manifesto.
- Implementação:
1. Promover mais subconjuntos estáveis de `tests/heavy` ao runner oficial (`nightly` ou `stress`), começando por segurança e stress reprodutíveis.
2. Publicar alvo de coverage com relatório simples por arquivo ou por módulo.
3. Revisar periodicamente se `hardening/concurrent_compilation` deve subir para `pr_gate`.
4. Manter a matriz do formatter alinhada às superfícies novas da linguagem e do manifesto.
- Testes necessários:
1. Execução contínua no runner oficial.
2. Gate de regressão específico para segurança/stress.
3. Relatório de coverage arquivado por release/nightly.
- Critério de conclusão:
1. Segurança pesada, fuzz semântico e formatter deixam de depender de cobertura ocasional.
2. Existe visibilidade objetiva do que ainda não está coberto.

---

## 4. Roadmap de bugfix

## R1 — Segurança e correção estrutural imediata

**Objetivo:** eliminar riscos de execução indevida, overflow e comportamento silencioso perigoso.

Itens:

- `BF-01` command injection em processo.
- `BF-02` overflow de alocação em concat/join.
- `BF-04` remover scaffolding morto de ciclos RC.
- `BF-06` leak em `zt_net_error_kind_index`.
- `BF-08` remover cap implícito de alta aridade.

Entregáveis:

- nova execução de processo sem shell;
- helpers de overflow centralizados;
- runtime limpo de código morto perigoso;
- regressões mínimas de segurança e memória.

Janela sugerida:

- 3 a 5 dias úteis.

## R2 — Semântica de runtime e portabilidade

**Objetivo:** estabilizar contrato de texto, ownership e compilação cross-compiler.

Itens:

- `BF-03` política explícita de thread-safety;
- `BF-05` fronteiras UTF-8;
- `BF-09` compatibilidade com MSVC.

Entregáveis:

- política oficial de thread-safety;
- semântica de texto documentada e implementada;
- build compatível em MSVC.

Janela sugerida:

- 4 a 7 dias úteis.

## R3 — Fechamento de superfície da stdlib

**Objetivo:** reduzir a diferença entre API declarada e backend real.

Itens:

- `STDLIB-03` fechado;
- `STDLIB-02` fechado no recorte alpha atual;
- `STDLIB-01` fechado por alinhamento de superfície ao subset seguro.

Entregáveis:

- `std.os`/`std.os.process` fechados;
- `std.fs` utilitário e confiável no recorte alpha atual;
- `std.text` rebaixado para subset honesto e testado.

Janela sugerida:

- concluida neste passe de fechamento.

## R4 — Alinhamento documental e normativo

**Objetivo:** deixar o repositório com uma história única sobre o que é suportado hoje.

Itens:

- `DOC-01` taxonomia de diagnósticos runtime;
- `DOC-02` alinhamento MVP/status/docs.

Entregáveis:

- docs sem conflito interno;
- catálogo e fixtures coerentes;
- regra editorial clara sobre precedência de documentos.

Janela sugerida:

- 2 a 4 dias úteis.

## R5 — Escalabilidade e hardening contínuo

**Objetivo:** atacar memória, custo assintótico e lacunas de teste.

Itens:

- `BF-10` emitter com streaming/flush;
- `TEST-01` trilhas de hardening.

Entregáveis:

- emissão C mais escalável;
- suíte de segurança, fuzz semântico, coverage e formatter fortalecida.

Janela sugerida:

- 2 a 3 semanas.

---

## 5. Checklist operacional

### Fase R1

- [x] remover uso de `system()` do caminho user-facing de `process.run`
- [x] introduzir execução segura por `argv` no host runtime
- [x] adicionar testes com args hostis e `cwd`
- [x] criar helpers de overflow de tamanho
- [x] aplicar guards em `zt_text_concat`
- [x] aplicar guards em `zt_bytes_join`
- [ ] revisar concatenações derivadas de path/buffer temporário
- [x] remover `zt_detect_cycles`
- [x] remover `zt_runtime_check_for_cycles`
- [x] eliminar leak em `zt_net_error_kind_index`
- [x] substituir `used_params[128]` por estrutura dinâmica
- [x] criar fixture de alta aridade

### Fase R2

- [x] decidir formalmente a política de thread-safety do runtime
- [x] documentar a política escolhida em spec e README
- [x] alinhar `MVP_OUT_OF_SCOPE.md` e `surface-implementation-status.md` ao modelo atual de ARC/isolate/cycles
- [x] migrar globals do driver para contexto explícito ou marcar como single-thread only
- [x] definir semântica oficial de index/slice para `text`
- [x] implementar fronteiras UTF-8 válidas ou rebaixar/documentar o contrato atual
- [x] alinhar queue/stack ao padrão de COW das demais coleções
- [x] criar testes de cópia + mutação para queue/stack
- [x] encapsular builtins de overflow por compilador
- [x] validar build em MSVC

### Fase R3

- [x] implementar `zt_host_os_args`
- [x] implementar `zt_host_process_run_capture`
- [x] fechar suíte de testes de `std.os.process`
- [x] implementar bloco inicial de `std.fs` query/metadados
- [x] implementar bloco de criação/remoção em `std.fs`
- [x] implementar bloco de cópia/movimentação em `std.fs`
- [x] cobrir `std.fs` com fixtures temporárias isoladas
- [x] entregar subset alpha seguro de `std.text` busca/comparação
- [x] entregar subset alpha seguro de `std.text` trim/predicados/UTF-8
- [x] remover da superfície alpha os helpers de `std.text` ainda não seguros
- [x] atualizar ZDocs e matriz de cobertura para o recorte entregue

### Fase R4

- [x] escolher a taxonomia runtime oficial
- [x] alinhar runtime, catálogo, spec e fixtures aos mesmos códigos
- [x] escolher documento canônico para status do corte atual
- [x] atualizar `MVP_OUT_OF_SCOPE.md`
- [x] atualizar `surface-implementation-status.md`
- [ ] revisar decisões históricas ainda não alinhadas fora deste bloco

### Fase R5

- [ ] introduzir writer/stream no emitter C
- [ ] medir pico de memória antes e depois do emitter streaming
- [x] decidir contrato real de performance para `map<text,text>` no alpha
- [x] implementar índice/hash real no runtime atual de `map<text,text>`
- [x] integrar subconjunto estável de `tests/heavy` ao gate oficial
- [x] criar harness de compilação concorrente
- [ ] publicar alvo de coverage
- [x] adicionar pelo menos 8 novos casos golden de formatter

### Relatórios Externos (22-04-2026)

Após cruzar `22-04-2026-gemma4-31b-implementation-report.md`, `22-04-2026-qwen3.5-plus-implementation-report.md` e `22-04-2026-qwen3.6-plus-implementation-report.md` com o código atual, os pontos que ainda se sustentam são:

1. ARC segue não atômico fora do contrato documentado de `single-isolate`.
2. parte do bloco `tests/heavy` ainda não é gate principal de hardening, embora a fuzz semântica e o harness concorrente já tenham sido promovidos para o runner oficial.

---

## 6. Fechamento

Este documento deve ser tratado como base operacional de correção.

Ele não tenta recontar toda a história do projeto.
Ele não reabre itens já refutados.
Ele não mistura bug ativo com drift documental sem diferenciar prioridade.

A regra daqui para frente deve ser simples:

- só entra neste backlog o que ainda precisa ser corrigido, decidido, implementado ou coberto por teste;
- cada item precisa ter dono, teste e critério de saída;
- cada release deve fechar ou reclassificar explicitamente os itens impactados.
