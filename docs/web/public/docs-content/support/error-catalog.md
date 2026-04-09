# Catálogo de Erros

O compilador Zenith é desenhado para ser um mentor. Aqui estão os erros mais comuns e como resolvê-los.

## Erros de Sintaxe (Parsing)

### `Z001: Expected 'end'`
O Zenith usa blocos limpos. Se você abrir um `if`, `func`, `struct` ou `loop`, você deve fechar com `end`.
- **Solução**: Verifique se todos os seus blocos têm um `end` correspondente.

### `Z002: Unexpected Token`
O compilador encontrou algo que não faz sentido naquela posição.
- **Solução**: Verifique se você não esqueceu dois pontos `:` ou se não usou `;` (que não existe no Zenith).

## Erros de Tipo (Semantic)

### `Z101: Type Mismatch`
Você tentou atribuir um tipo a uma variável que espera outro.
- **Exemplo**: `var x: int = "texto"`
- **Solução**: Certifique-se de que os tipos coincidem ou use uma união `int | text`.

### `Z102: Missing Type Annotation`
Variáveis globais e parâmetros de função no Zenith exigem tipos explícitos.
- **Solução**: Adicione `: Tipo` após o nome da variável.

## Erros de Reatividade

### `Z201: Cyclic Dependency`
Você criou um `computed` ou `watch` que altera a si mesmo ou cria um loop infinito de atualizações.
- **Solução**: Simplifique a lógica para evitar que um estado dependa circularmente de outro.
