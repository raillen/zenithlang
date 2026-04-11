# Especificação Técnica: Sistema de Tipos

Este documento detalha a fundação estática do Zenith. O sistema de tipos foi projetado para garantir a soberania técnica do desenvolvedor, permitindo que erros de lógica sejam capturados antes da execução.

## 1. Tipos Primitivos

| Tipo | Descrição | Motivação |
| :--- | :--- | :--- |
| `int` | Inteiro de 64 bits | Precisão absoluta para IDs e contadores. |
| `float` | Ponto flutuante | Cálculos físicos e posicionamento espacial. |
| `text` | Sequência UTF-8 | Manipulação de texto com suporte a múltiplos idiomas. |
| `bool` | Valor lógico | Tomada de decisão binária (`true` ou `false`). |
| `any` | Tipo Dinâmico | Escape hatch para interoperabilidade com o ecossistema Lua. |
| `null` | Ausência de dado | Representação de valor inexistente (evite o uso direto). |
| `void` | Retorno vazio | Indica que uma função não produz dado de saída. |

---

## 2. Tipos de Segurança (ADTs)

Zenith substitui convenções perigosas por estruturas formais de segurança.

### O Tipo `Optional<T>`
Representa a incerteza de existência.
*   **Present(T)**: A caixa contém o valor.
*   **Empty**: A caixa está vazia.

### O Tipo `Outcome<T, E>`
Representa o resultado de uma operação que pode falhar.
*   **Success(T)**: A operação foi concluída.
*   **Failure(E)**: A operação falhou com o erro informado.

---

## 3. Apelidos Semânticos (`type`)

A palavra-chave `type` permite criar um nome alternativo para um tipo existente. 

*   **Regra**: Atualmente os apelidos são transparentes (Structural Typing).
*   **Uso**: Serve para dar significado ao dado (ex: `type Preco = float`).
*   **Custo**: Zero. O apelido desaparece após a compilação.

---

## 4. Modelagem de Dados

### Estruturas (`struct`)
Define um objeto com campos nomeados.
```zt
struct Item
    pub nome: text
    pub peso: float where it > 0.0
end
```
*   **Cláusula `where`**: Permite definir um contrato de integridade. O Zenith garante que o dado nunca viole esta regra.

### Tipos Soma (`enum`)
Define que um dado pode assumir uma de várias formas, podendo inclusive carregar dados extras.
```zt
enum Estado
    Carregando
    Erro(text)
    Pronto(list<int>)
end
```

---

## 5. Contratos de Comportamento (`trait`)

Traits definem o que um tipo **pode fazer**, em vez do que ele **é**.

1.  **Definição**: Declare os métodos necessários.
2.  **Aplicação**: Use `apply Trait for Struct` para implementar o contrato.
3.  **Vantagem**: Permite polimorfismo seguro e reutilização de código sem a fragilidade da herança clássica.

---
*Zenith Specification v0.3.1 - Sistema de Tipos*
