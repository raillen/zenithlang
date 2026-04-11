# Mapa de Intenções Visuais

Este documento serve como o dicionário definitivo de todos os símbolos e operadores da linguagem Zenith. Ele define como o cérebro do programador deve interpretar cada sinal gráfico para reduzir a carga cognitiva.

## 1. Símbolos de Estrutura e Bloco

| Símbolo | Nome | Intenção (O que o cérebro deve ler) |
| :--- | :--- | :--- |
| `do` | Gatilho | "Prepare-se: a ação definida anteriormente vai começar agora." |
| `end` | Âncora | "Pare: o contexto atual foi concluído e isolado." |
| `:` | Escopo | "Atenção: o que vem a seguir é uma lista de instruções ou uma definição de tipo." |
| `=>` | Mapeamento | "Transformação: o valor da esquerda resulta no valor da direita." |
| `->` | Retorno | "Saída: esta função ou operação produz este tipo de dado." |

---

## 2. Operadores de Fluxo e Dados

| Símbolo | Nome | Intenção (O que o cérebro deve ler) |
| :--- | :--- | :--- |
| `?` | Propagador | "Talvez: se houver um erro ou valor vazio, pare e passe para quem me chamou." |
| `!` | Extrator | "Certeza: eu afirmo que este valor existe. Se eu estiver errado, interrompa tudo." |
| `..` | Conector | "Sequência: todos os valores entre o início e o fim, ou todos os itens desta lista." |
| `@` | Atributo | "Metadado: esta regra ou valor pertence ao contexto interno do objeto ou do compilador." |
| `#` | Medidor | "Contagem: qual é o tamanho total ou a quantidade de itens desta estrutura?" |

---

## 3. Comparações e Lógica

| Símbolo | Nome | Intenção (O que o cérebro deve ler) |
| :--- | :--- | :--- |
| `=` | Atribuição | "Armazenar: o nome da esquerda agora aponta para o valor da direita." |
| `==` | Igualdade | "Verificar: o valor da esquerda é idêntico ao da direita?" |
| `!=` | Diferença | "Verificar: o valor da esquerda é estranho ao da direita?" |
| `is` | Identidade | "Contrato: este objeto segue as regras deste Tipo ou Trait?" |

---

## 4. Diferenças Semânticas Críticas (Fim da Ambiguidade)

### Diferença entre `:` e `=>`
*   **Use `:`** quando você estiver abrindo uma lista de comandos (Modo Procedural). O `:` pede um recuo de linha e foca em **O que fazer**.
*   **Use `=>`** quando você estiver transformando um valor em outro (Modo Funcional). O `=>` geralmente termina na mesma linha e foca em **O que retornar**.

### Diferença entre `->` e `:` em Funções
*   **Use `->`** para definir o tipo de retorno final da função.
*   **Use `:`** dentro de parênteses para definir o tipo de um parâmetro individual.

---
*Zenith Specification v0.3.1 - Mapa de Intenções*
