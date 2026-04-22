# Relatório Operacional de Pendências — Zenith Lang v2

**Data:** 2026-04-22  
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
| P0 | 2 | execução de processo via shell, overflow em alocação |
| P1 | 7 | thread-safety, ARC, UTF-8, leak, COW, validação de alta aridade, portabilidade |
| P2 | 7 | escalabilidade do emitter, mapa linear, drift de docs/spec, lacunas de stdlib |
| P3 | 1 | hardening sistemático de testes |

### Ordem recomendada

1. Fechar primeiro o que pode gerar execução indevida, overflow ou comportamento perigoso silencioso.
2. Em seguida, estabilizar semântica de runtime e portabilidade.
3. Depois, completar bindings de stdlib e alinhar documentação normativa.
4. Por fim, atacar performance estrutural e ampliar hardening de testes.

---

## 3. Backlog operacional

### BF-01 — Command injection em `std.os.process.run`
- Prioridade: `P0`
- Estado: `Confirmado`
- Área: `Runtime / Segurança`
- Evidência: `zt_host_default_process_run` monta uma string com programa + argumentos e executa `system(command)`.
- Risco: qualquer argumento com metacaracteres de shell pode mudar o comando executado.
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
- Estado: `Confirmado`
- Área: `Runtime / Segurança / Corretude`
- Evidência: somas de comprimentos são feitas antes da alocação sem guarda explícita.
- Risco: overflow de `size_t`, alocação menor que a necessária e escrita fora do buffer.
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
- Estado: `Confirmado`
- Área: `Runtime / Driver / Arquitetura`
- Evidência: `zt_retain` e `zt_release` não são atômicos; `zt_last_error` e globals do driver não têm isolamento por contexto.
- Risco: se a base evoluir para paralelismo real, surgem double-free, UAF, vazamento de estado e resultados não determinísticos.
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
- Estado: `Confirmado`
- Área: `Runtime / Higiene`
- Evidência: `zt_detect_cycles` altera o bit alto de `rc`; `zt_runtime_check_for_cycles` é stub; o bloco não participa do fluxo normal.
- Risco: código morto perigoso confunde auditoria e pode ser religado de forma insegura no futuro.
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
- Estado: `Confirmado`
- Área: `Runtime / Semântica`
- Evidência: indexação e slice operam por byte e podem cortar no meio de sequência multi-byte.
- Risco: textos inválidos saem do runtime e quebram invariantes da linguagem.
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
- Estado: `Confirmado`
- Área: `Runtime / Memória`
- Evidência: a função cria vários `zt_text_from_utf8_literal(...)` por chamada sem liberar.
- Risco: vazamento repetitivo em caminhos de erro de rede.
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
- Estado: `Confirmado`
- Área: `Runtime / Coleções`
- Evidência: `dequeue` e `pop` mutam a estrutura diretamente, ao contrário do padrão `*_owned` já usado em outras coleções.
- Risco: semântica inconsistente de ownership e isolamento após cópia/compartilhamento.
- Implementação:
1. Definir contrato unificado de mutação para queue/stack.
2. Introduzir variantes `*_dequeue_owned` e `*_pop_owned`, ou tornar as operações atuais COW-aware.
3. Se a API atual for mantida, alinhar toda a documentação de ownership de coleções.
4. Cobrir queue/stack com o mesmo padrão de isolamento já usado em outras coleções gerenciadas.
- Testes necessários:
1. Compartilhar queue/stack e mutar cada cópia.
2. Casos com tipos simples e `text`.
3. Casos vazios e com múltiplos elementos.
- Critério de conclusão:
1. Queue/stack seguem a mesma regra de ownership das demais coleções do runtime.
2. Há regressão E2E cobrindo cópia e mutação isolada.

### BF-08 — Validação de chamadas com alta aridade depende de array fixo e retorna cedo
- Prioridade: `P1`
- Estado: `Confirmado`
- Área: `Typechecker / Corretude`
- Evidência: `used_params[128]` foi protegido contra overflow, mas a proteção atual apenas retorna cedo e pode deixar o caso sem diagnóstico claro.
- Risco: chamadas com muitos parâmetros podem escapar da validação esperada ou produzir resultado silenciosamente incompleto.
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
- Estado: `Confirmado`
- Área: `Runtime / Portabilidade`
- Evidência: builtins GNU são usados diretamente nas operações aritméticas de overflow.
- Risco: build falha no Windows com MSVC, apesar de o projeto mirar ambiente Windows.
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
- Estado: `Confirmado`
- Área: `Backend C / Performance`
- Evidência: o emitter acumula tudo em memória antes de persistir resultado.
- Risco: pico de memória cresce com tamanho do módulo e limita projetos maiores.
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
- Estado: `Confirmado`
- Área: `Runtime / Performance`
- Evidência: operações seguem caminho linear e a infraestrutura de hash não está fechada como benefício real para o caminho genérico.
- Risco: degradação visível em mapas grandes e falsa sensação de complexidade melhor que O(n).
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

### DOC-01 — Taxonomia de diagnósticos runtime diverge da spec
- Prioridade: `P2`
- Estado: `Confirmado`
- Área: `Spec / Diagnostics / Tooling`
- Evidência: a spec exige códigos como `runtime.contract`, `runtime.bounds`, `runtime.map_key`, `runtime.utf8`, `runtime.divide_by_zero`, enquanto o catálogo e o runtime expõem nomes diferentes.
- Risco: ferramentas, LSP, docs e testes podem falar línguas diferentes sobre o mesmo erro.
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
- Estado: `Confirmado`
- Área: `Documentação / Governança`
- Evidência: multifile, `.ztproj` e ZDoc aparecem como post-MVP em um lugar e conformantes em outro.
- Risco: roadmap errado, auditoria errada e contribuições desalinhadas.
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

### STDLIB-01 — `std.text` declara 25 externs sem backend correspondente no runtime
- Prioridade: `P2`
- Estado: `Confirmado`
- Área: `Stdlib / Runtime`
- Funções pendentes:
1. `zt_text_trim`
2. `zt_text_trim_start`
3. `zt_text_trim_end`
4. `zt_text_contains`
5. `zt_text_starts_with`
6. `zt_text_ends_with`
7. `zt_text_has_whitespace`
8. `zt_text_index_of`
9. `zt_text_last_index_of`
10. `zt_text_replace_all`
11. `zt_text_replace_first`
12. `zt_text_split`
13. `zt_text_split_lines`
14. `zt_text_join`
15. `zt_text_is_empty`
16. `zt_text_to_lower`
17. `zt_text_to_upper`
18. `zt_text_capitalize`
19. `zt_text_truncate`
20. `zt_text_limit`
21. `zt_text_pad_left`
22. `zt_text_pad_right`
23. `zt_text_mask`
24. `zt_text_title_case`
25. `zt_text_is_digits`
- Risco: a superfície pública sugere capacidade que o backend ainda não entrega por completo.
- Implementação:
1. Separar o backlog em três lotes: busca/comparação, trim/split/join, transformação/format helpers.
2. Implementar primeiro o que pode ser escrito em Zenith puro com primitives existentes, se isso reduzir risco e tempo.
3. Implementar em C apenas o que exigir performance, UTF-8 cuidadoso ou integração direta com bytes.
4. Adicionar testes por função e por grupo semântico.
5. Atualizar ZDoc e matriz de cobertura ao final de cada lote.
- Testes necessários:
1. Casos ASCII e UTF-8.
2. Casos vazios e extremos.
3. Split/join com separador vazio e múltiplas ocorrências.
4. Transformações de casing e padding.
- Critério de conclusão:
1. Toda função declarada em `std.text` tem backend ou implementação equivalente entregue.
2. ZDoc, behavior tests e docs batem com o que existe de verdade.

### STDLIB-02 — `std.fs` declara 15 externs sem backend correspondente no runtime
- Prioridade: `P2`
- Estado: `Confirmado`
- Área: `Stdlib / Host API`
- Funções pendentes:
1. `zt_host_fs_append_text`
2. `zt_host_fs_copy_file`
3. `zt_host_fs_create_dir`
4. `zt_host_fs_create_dir_all`
5. `zt_host_fs_created_at`
6. `zt_host_fs_is_dir`
7. `zt_host_fs_is_file`
8. `zt_host_fs_list`
9. `zt_host_fs_metadata`
10. `zt_host_fs_modified_at`
11. `zt_host_fs_move`
12. `zt_host_fs_remove_dir`
13. `zt_host_fs_remove_dir_all`
14. `zt_host_fs_remove_file`
15. `zt_host_fs_size`
- Risco: a stdlib de filesystem fica parcial e inconsistente em relação à API declarada.
- Implementação:
1. Criar camada host FS mínima e portátil com wrappers separados para Windows e POSIX.
2. Definir mapeamento estável de erros para `std.fs.Error`.
3. Entregar primeiro query/metadados, depois criação/remoção, depois cópia/movimentação e append.
4. Fechar comportamento para paths inexistentes, permissões, diretório vs arquivo e timestamps.
- Testes necessários:
1. Fixtures temporárias em diretório isolado.
2. Casos de arquivo, diretório, inexistente e permissão negada.
3. Timestamps e metadata com resultado tipado.
4. Testes de limpeza para evitar lixo entre execuções.
- Critério de conclusão:
1. `std.fs` deixa de anunciar funções sem backend.
2. A suíte cobre create/list/metadata/remove/copy/move em caminhos reais.

### STDLIB-03 — lacunas restantes em `std.os` e `std.os.process`
- Prioridade: `P2`
- Estado: `Confirmado`
- Área: `Stdlib / Runtime / Host API`
- Funções pendentes:
1. `zt_host_os_args`
2. `zt_host_process_run_capture`
- Risco: `std.os` e `std.os.process` parecem mais completos do que realmente estão.
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
- Estado: `Confirmado`
- Área: `Qualidade / Testes`
- Lacunas a fechar:
1. regressões de segurança para execução de processos;
2. fuzzing da camada semântica e não só lexer/parser;
3. testes de paralelismo do compilador e isolamento de contexto;
4. infraestrutura básica de cobertura;
5. expansão dos golden tests do formatter além de `case_all`.
- Implementação:
1. Criar suíte `tests/security/` para processo, paths hostis e inputs grandes.
2. Adicionar mutadores e seeds semânticos ao fuzz atual.
3. Criar harness de compilação concorrente com múltiplos projetos e múltiplas flags.
4. Publicar alvo de coverage com relatório simples por arquivo ou por módulo.
5. Adicionar pelo menos 8 novos casos de formatter cobrindo imports, structs, match, generics, comments, multiline strings, trailing commas e manifests.
- Testes necessários:
1. Execução contínua no runner oficial.
2. Gate de regressão específico para segurança.
3. Relatório de coverage arquivado por release/nightly.
- Critério de conclusão:
1. Segurança, semântica e formatter deixam de depender de cobertura ocasional.
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
- `BF-07` COW de queue/stack;
- `BF-09` compatibilidade com MSVC.

Entregáveis:

- política oficial de thread-safety;
- semântica de texto documentada e implementada;
- queue/stack coerentes com o restante das coleções;
- build compatível em MSVC.

Janela sugerida:

- 4 a 7 dias úteis.

## R3 — Fechamento de superfície da stdlib

**Objetivo:** reduzir a diferença entre API declarada e backend real.

Itens:

- `STDLIB-03` primeiro, porque depende do novo executor seguro de processo;
- `STDLIB-02` em seguida, por impacto de tooling e projetos reais;
- `STDLIB-01` em lotes internos bem definidos.

Entregáveis:

- `std.os`/`std.os.process` fechados;
- `std.fs` utilitário e confiável;
- backlog de `std.text` dividido em entregas pequenas e testáveis.

Janela sugerida:

- 2 a 4 semanas, dependendo da profundidade de UTF-8 e cross-platform.

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
- `BF-11` mapa genérico e custo real;
- `TEST-01` trilhas de hardening.

Entregáveis:

- emissão C mais escalável;
- contrato honesto de performance para `map<K,V>`;
- suíte de segurança, fuzz semântico, coverage e formatter fortalecida.

Janela sugerida:

- 2 a 3 semanas.

---

## 5. Checklist operacional

### Fase R1

- [ ] remover uso de `system()` do caminho user-facing de `process.run`
- [ ] introduzir execução segura por `argv` no host runtime
- [ ] adicionar testes com args hostis e `cwd`
- [ ] criar helpers de overflow de tamanho
- [ ] aplicar guards em `zt_text_concat`
- [ ] aplicar guards em `zt_bytes_join`
- [ ] revisar concatenações derivadas de path/buffer temporário
- [ ] remover `zt_detect_cycles`
- [ ] remover `zt_runtime_check_for_cycles`
- [ ] eliminar leak em `zt_net_error_kind_index`
- [ ] substituir `used_params[128]` por estrutura dinâmica
- [ ] criar fixture de alta aridade

### Fase R2

- [ ] decidir formalmente a política de thread-safety do runtime
- [ ] documentar a política escolhida em spec e README
- [ ] migrar globals do driver para contexto explícito ou marcar como single-thread only
- [ ] definir semântica oficial de index/slice para `text`
- [ ] implementar fronteiras UTF-8 válidas ou rebaixar/documentar o contrato atual
- [ ] alinhar queue/stack ao padrão de COW das demais coleções
- [ ] criar testes de cópia + mutação para queue/stack
- [ ] encapsular builtins de overflow por compilador
- [ ] validar build em MSVC

### Fase R3

- [ ] implementar `zt_host_os_args`
- [ ] implementar `zt_host_process_run_capture`
- [ ] fechar suíte de testes de `std.os.process`
- [ ] implementar bloco inicial de `std.fs` query/metadados
- [ ] implementar bloco de criação/remoção em `std.fs`
- [ ] implementar bloco de cópia/movimentação em `std.fs`
- [ ] cobrir `std.fs` com fixtures temporárias isoladas
- [ ] entregar lote 1 de `std.text` busca/comparação
- [ ] entregar lote 2 de `std.text` trim/split/join
- [ ] entregar lote 3 de `std.text` transformações e helpers de apresentação
- [ ] atualizar ZDocs e matriz de cobertura por lote entregue

### Fase R4

- [ ] escolher a taxonomia runtime oficial
- [ ] alinhar runtime, catálogo, spec e fixtures aos mesmos códigos
- [ ] escolher documento canônico para status do corte atual
- [ ] atualizar `MVP_OUT_OF_SCOPE.md`
- [ ] atualizar `surface-implementation-status.md`
- [ ] revisar conformance matrix, README e decisões afetadas

### Fase R5

- [ ] introduzir writer/stream no emitter C
- [ ] medir pico de memória antes e depois do emitter streaming
- [ ] decidir contrato real de performance para `map<K,V>`
- [ ] implementar índice/hash real ou documentar custo atual
- [ ] criar suíte `tests/security/`
- [ ] ampliar fuzzing para a camada semântica
- [ ] criar harness de compilação concorrente
- [ ] publicar alvo de coverage
- [ ] adicionar pelo menos 8 novos casos golden de formatter

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
