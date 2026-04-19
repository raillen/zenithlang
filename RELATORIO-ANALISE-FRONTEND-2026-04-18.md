# Relatório de Análise: Frontend do Compilador Zenith (`compiler/frontend`)

Este relatório apresenta uma análise aprofundada do módulo `compiler/frontend` da linguagem Zenith, cobrindo o Lexer, Parser e a AST. O objetivo é destacar os pontos fortes da arquitetura atual, apontar débitos técnicos e gargalos, além de recomendar um plano tático de melhorias.

## 1. Visão Geral da Arquitetura

O frontend é composto por um pipeline clássico:
- **Lexer** (`lexer.c/h` e `token.c/h`): Transforma texto em tokens de forma preditiva (sem regex), lidando com literais básicos e triviais em uma única varredura.
- **AST** (`ast/model.c/h`): Representa a árvore sintática gerada, usando *Tagged Unions* (`zt_ast_kind` + `union` com structs anônimas ou nomeadas) em nós alocados dinamicamente na Heap.
- **Parser** (`parser.c/h`): Parser de Descida Recursiva (Recursive Descent Parser) construído nativamente à mão, que consome o Lexer e constrói a AST. 

---

## 2. Coisas Boas (Aspectos Positivos)

O código apresenta excelentes práticas para uma linguagem em evolução:

1. **Parser de Descida Recursiva Limpo:** O parser feito à mão não depende de ferramentas geradoras (como Yacc/Bison). Isso facilita demais a extensão da linguagem. Adicionar uma nova sintaxe de if/match com payload é linear: basta ler os novos tokens e montar o nó.
2. **Recuperação de Erro Contextual (Error Recovery):** A existência de `zt_parser_sync_to_declaration()` e sugestões como `"use '->' for return type"` demonstram maturidade no Design da Experiência do Usuário (UX do compilador). O parser tenta sobreviver a erros sintáticos para reportar múltiplos problemas na mesma compilação em vez de morrer no primeiro erro fatal.
3. **Tracking de Spans Sólido:** Todo AST node e Token armazena o correspondente `zt_source_span` (linha, coluna de início/fim). Isso garante mensagens de erro excelentes no `zt_diag_list` sem perda da localização original na etapa de lowering C.
4. **Sem Alocações Lentas no Lexer (Parcialmente):** A ausência de `malloc` para alocar strings individuais durante a tokenização (salvo quando vira AST) e o bypass de comentários sem processamento profundo é muito rápido na varredura linear.

---

## 3. Coisas Ruins / Pontos de Atenção (Críticas)

Embora o design inicial seja robusto, existem armadilhas escondidas que podem explodir conforme o ecossistema progrida em escala:

### 3.1. Gargalo de Memória Severo no Lexer (`sizeof(zt_token)`)
Em `token.h`, a struct `zt_token` define `char text[ZT_TOKEN_MAX_TEXT];` onde `ZT_TOKEN_MAX_TEXT` é 1024 bytes.
- Isso significa que **todo token possui estaticamente mais de 1 KB** de tamanho na stack ou heap associada ao Parser.
- Como o parser faz *copy* do token inteiro no lookahead (`p->peek` e `p->current`), um swap de tokens chega a copiar múltiplos Kilobytes na CPU l-cache repetidas vezes. A explosão do tamanho do token deteriora localidade de cache severamente.

### 3.2. Strings Maiores que 1024 Bytes Causam Truncamento (Bugs silenciosos/fatais)
Qualquer string literal, texto com triple quotes explícito (`"""`), ou código binário via `hex bytes` que ultrapasse 1024 caracteres será agressivamente truncado gerando `ZT_DIAG_TOKEN_TOO_LONG`. Em uma linguagem com foco em jogos ou em automação (onde strings gigantes JSON inteiras são coladas no código fonte), isso limitará gravemente seu uso prático.

### 3.3. Fragmentação Extrema de AST na Heap
Cada `zt_ast_make` invoca um `malloc()` ou equivalente. Isso incorre em dois problemas: 
- Sobrecarga e perda de velocidade real pela alocação unitária e descentralizada da memória para centenas de milhares de nós em arquivos grandes.
- Custos pesados de varredura no `zt_ast_dispose` onde a árvore é percorrida recursivamente para dar `free()`.

### 3.4. Estouro de Stack no Parser (OOM/Crash local)
Sendo puramente recursivo e construindo ASTs complexas lado a lado com os nós (ex: blocos muito longos ou cadeias repetitivas de propriedades `a.b.c.d.e...`), ainda há um risco pequeno de Stack Overflow do sistema operacional antes mesmo do AST ser formado. A mitigação `ZT_PARSER_STACK_BUFFER_THRESHOLD` ajuda no parse de texto temporário, mas não blinda recursões em chamadas encadeadas.

---

## 4. Recomendações e Plano de Ação

Aqui está a receita de como evoluir o Frontend preparando-o para o momento enxuto (quando começarmos a trabalhar com grandes fluxos de código provenientes de ZPM / Stdlib ou scripts complexos):

### Recomendação A: Implementação de um "String Interning Pool" e `string_view`
**Por que:** Corrige a limitação dos 1024 bytes e faz a memória/cache rodar a picos altíssimos.
**Como Implementar:**
1. Altere o `zt_token` no `token.h` removendo `char text[1024];`.
2. Substitua pelo conceito de uma View de string (`const char *start_ptr; uint32_t length;`) que aponte diretamente para o buffer imutável nativo (`source_text`) carregado da memória do arquivo original. 
3. Quando precisarmos do buffer persistente para nós que exigem cópia limpa (ex: com escapes interpolados de string literais), coloque a string final tratada dentro de conjunto global por Thread/Lexer (um Memory Pool), de forma que múltiplas strings iguais apontem para a mesma região da memória prevenida de double-frees (String Interning).

### Recomendação B: Integração de Arenas (Bump Allocator) para a AST
**Por que:** `malloc` individual para AST em C destrói a performance global para milhões de linhas de código.
**Como Implementar:**
1. Crie uma estrutura global de transição `zt_arena_allocator` que solicite grandes blocos à SO (p.ex. chunks de 64MB por vez via `mmap` / `VirtualAlloc`).
2. Mude todas as assinaturas `zt_ast_make(...)` para `zt_ast_make(zt_arena *arena, ...)`.
3. Todo nó novo deve consumir offsets do ponteiro dessa arena. Assim sendo, a AST será contígua ou estará em pouquíssimas regiões.
4. O `zt_ast_dispose()` será completamente extinto. Quando o compilador finalizar aquele arquivo/pipeline, varremos a Arena destruindo as grandes páginas de uma vez. A complexidade O(N) do free se transformará em O(1).

### Recomendação C: Expandir o Modelo Lexer/Parser para Interpolação Segura (Strings interpoladas)
**Por que:** Melhoria ergonômica crítica faltante na surface nativa para os devs (Apesar da `std.format` ser prometida no runtime, construtores textuais fluidos salvam vidas).
**Como Implementar:**
1. Permitir tokens de interpolação tipo `\{` no scanner de StringLiteral (`lexer.c`).
2. Quando encontrado, o Lexer retorna um novo token `ZT_TOKEN_STRING_PART` em vez do literal inteiro. 
3. O parser processará esses cortes até o fechamento consumindo um nó de concatenação especial transparente na AST `ZT_AST_FORMAT_STRING_EXPR` baixando diretamente na RC otimizada do C gerado.

### Recomendação D: Extensão Fina sobre os Enums com Payload
Como já mencionado nos Roadmaps, a funcionalidade `match exaustivo` de payloads sobre os modulos (Bloqueadores M27) terá dependência direta não apenas na HIR -> ZIR, ela afeta primariamente o design da AST:
1. Precisamos introduzir na hierarquia AST `model.c` estruturas como `ZT_AST_MATCH_BINDING` (ex: `case Success(let value):`), já garantindo que o parser mapeie os capturados localmente no `binder.c`.
2. Expansão dos Diagnostics (`zt_parser_sync_to_...`): Inclusão de sincagens contextuais quando houver quebra de padronização no Enum Branch do Parser, guiando o erro com avisos como *"You cannot bind values outside a 'let' enum scope here."*
