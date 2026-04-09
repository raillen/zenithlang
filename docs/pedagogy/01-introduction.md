# Introdução à Engenharia de Linguagens

Bem-vindo à série pedagógica do Zenith. Aqui, não apenas ensinamos como usar a linguagem, mas como ela foi construída do zero.

## O que é um Compilador?
Um compilador é um tradutor de sonhos técnicos. Ele pega um texto escrito por um humano (Código Fonte) e o transforma em algo que a máquina entende (Código de Máquina ou, no caso do Zenith, Bytecode Lua).

### As Etapas do Zenith
O Zenith segue um pipeline clássico e robusto:

1.  **Lexer (Análise Léxica)**: Transforma o texto em uma lista de "Tokens" (átomos da linguagem).
2.  **Parser (Análise Sintática)**: Organiza os tokens em uma árvore chamada **AST** (Abstract Syntax Tree).
3.  **Semantic Analyzer (Binding)**: Verifica se as variáveis existem e se os tipos fazem sentido.
4.  **Codegen (Geração de Código)**: Transforma a AST em código Lua otimizado.

## Por que Zenith?
O Zenith foi criado para resolver a dissonância entre a diversidade cognitiva humana e a rigidez das linguagens tradicionais. Nossa arquitetura é modular, baseada em **Composição sobre Herança**, tornando-a ideal para aprendizado e extensibilidade.

---
> [!TIP]
> **Dica de Estudo**: Siga esta trilha na ordem sugerida. Cada capítulo constrói sobre o conhecimento do anterior.
