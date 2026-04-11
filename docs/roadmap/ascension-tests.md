# Plano de Testes Ascension: Rumo à Soberania

Este documento define a estratégia de validação exaustiva para o motor Ascension utilizando o framework ZTest. O objetivo é garantir que cada fase do compilador nativo seja testada contra casos de sucesso, falhas sintáticas e semânticas, e performance.

## 1. Fase 1: Genesis (Lexer)
Objetivo: Validar a quebra do código-fonte em fluxos de tokens rítmicos.

- [X] Literais Complexos: Verificado (phase1_lexer_test.zt).
    - [X] Inteiros, Floats (incluindo notação científica 1.5e-10).
    - [X] Strings com interpolação aninhada: "Valor: {obj.get_val()}".
    - [X] Strings multi-linha (Raw Strings).
- [X] Operadores e Delimitadores: Verificado.
    - [X] Operadores compostos: +=, -=, .., ->, =>.
    - [X] Símbolos especiais: ?, !, @, #.
- [X] Preservação de Layout: Verificado (v010_exhaustive_battery_test.zt).
    - [X] Verificação de Span (linha/coluna) para garantir diagnósticos precisos na Fase 9.

---

## 2. Fase 2 e 3: Estrutura e Blocos (AST e Parser)
Objetivo: Garantir que a árvore sintática represente fielmente a lógica do código.

- [X] Controle de Fluxo (Stress Test): Verificado (phase2_3_parser_test.zt).
    - [X] Aninhamento profundo de if/elif/else.
    - [X] Loops while e for com declarações internas.
    - [X] match expressions com guardas: case x if x > 10 => ....
- [X] Definições de Tipos: Verificado.
    - [X] Structs com campos opcionais.
    - [X] Enums com variantes complexas.
    - [X] Traits e assinaturas de métodos.
- [X] Recuperação de Erro: Verificado.
    - [X] Testar se o parser consegue continuar após encontrar um erro (Panic Mode Recovery).

---

## 3. Fase 4: Identidade e Contexto (Binding)
Objetivo: Validar a resolução de nomes, escopos e tipos.

- [X] Escopo de Variáveis: Verificado (phase4_binding_test.zt).
    - [X] Shadowing de variáveis globais e locais.
    - [X] Acesso a self dentro de métodos de struct e blocks apply.
- [X] Resolução de Símbolos: Verificado.
    - [X] Chamadas de funções antes e depois de suas definições.
    - [X] Verificação de visibilidade (pub vs privado).
- [X] Inferência de Tipos: Verificado (v010_exhaustive_battery_test.zt).
    - [X] Validação de tipos em atribuições complexas e retornos de função.

---

## 4. Fase 5 e 10: Emitter e Transcendência (Codegen)
Objetivo: Garantir que o código Lua gerado seja correto e otimizado.

- [X] Snapshot Testing (Crítico): Verificado (phase5_emitter_test.zt).
    - [X] Usar assert_snapshot para comparar a saída Lua byte-a-byte com padrões de referência.
    - [X] Validar que o código gerado não contém variáveis globais Lua acidentais.
- [X] Performance de Execução: Verificado.
    - [X] Comparar o tempo de execução do código transpilado vs Lua escrito à mão.
- [X] Compilação Multi-Plataforma: Verificado (phase10_transcendence_test.zt).
    - [X] Atributos @windows/@linux eliminando código morto em tempo de build.
- [X] FFI Soberano: Verificado.
    - [X] Sintaxe 'extern func' integrada ao pipeline Ascension.

---

## 5. Fase 6 e 7: Autonomia e CLI (Módulos)
Objetivo: Validar o ecossistema de arquivos e comandos.

- [X] Resolução de Módulos: Verificado (phase6_7_cli_test.zt).
    - [X] Imports circulares (detecção e erro amigável).
    - [X] Namespaces aninhados: std.fs.path.
- [X] Build System: Verificado.
    - [X] Testar comando zt build com múltiplos arquivos.
    - [X] Validação de dependências no zenith.ztproj.

---

## 6. Fase 8: Soberania Nativa
Objetivo: Integração perfeita com Lua e Tipagem Genérica.

- [X] Native Lua Escape: Verificado (engine_integration_test.zt).
    - [X] Testar native lua como expressão embutida.
    - [X] Testar passagem de variáveis Zenith para blocos nativos e vice-versa.
- [X] Generics e Bounds: Verificado.
    - [X] Instanciação de Optional<T> e Outcome<T, E>.
    - [X] Verificação de restrições de Traits.

---

## 7. Fase 9: A Voz do Motor (Diagnostics)
Objetivo: Testar a clareza e a precisão das mensagens de erro.

- [X] Catálogo de Erros: Verificado (v010_exhaustive_battery_test.zt).
    - [X] Garantir que cada erro tenha uma dica e aponte para o Span exato.
- [X] Estética Aura: Verificado.
    - [X] Validar a colorização ANSI das mensagens no terminal.

---
*Atualizado em: 11 de Abril de 2026*
