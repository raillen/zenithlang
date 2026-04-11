# Idiomatismo e Boas Práticas: O Padrão Zenith

Este documento define como escrever código Zenith industrial. Seguir estes padrões garante que seu código seja legível por qualquer outro desenvolvedor e otimizado pelo compilador Ascension.

## 1. Nomenclatura e Intenção

Zenith utiliza o padrão visual para diferenciar dados de estruturas.

| Elemento | Padrão | Exemplo |
| :--- | :--- | :--- |
| Variáveis e Funções | `snake_case` | `velocidade_atual` |
| Constantes | `SCREAMING_SNAKE` | `GRAVIDADE_MAX` |
| Tipos e Estruturas | `PascalCase` | `PlayerProfile` |

---

## 2. Tipagem: Clareza sobre Adivinhação

*   **Contexto**: O compilador pode inferir tipos, mas o cérebro humano precisa de ancoras.
*   **A Regra**: Declare o tipo explicitamente em variáveis de nível de módulo ou funções públicas.
*   **O Contraste**: Diferente do JavaScript ou Lua, onde o tipo é uma surpresa, no Zenith o tipo é um contrato.

```zt
-- RECOMENDADO
var nivel: int = 1

-- EVITE (em código público)
var nivel = 1
```

---

## 3. Desembrulho Seguro (Safety First)

Este é o ponto mais crítico de estabilidade industrial do Zenith.

*   **Contexto**: Lidar com valores nulos ou falhas.
*   **A Regra**: Prefira sempre o operador `or` (valor padrão) ou `match`. Use o operador `!` apenas após uma verificação de `is_present()`.
*   **O Contraste**: O Zenith desencoraja o "Crash Orientado" (usar `!` em tudo), promovendo a resiliência.

```zt
-- ✅ SEGURO: Código industrial
var user = os.get_env("USER") or "admin"

-- ❌ PERIGOSO: Pode causar Panic no sistema
var user = os.get_env("USER")!
```

---

## 4. Composição sobre Herança

*   **A Regra**: Não tente criar hierarquias de classes. Crie dados (`struct`) e anexe comportamentos (`trait`).
*   **Vantagem**: Evita o "Problema do Diamante" e torna o código 100% testável individualmente.

---

## 5. Limites do Nativo (Boundary Principle)

*   **Contexto**: Zenith permite usar Lua nativo, mas isso quebra a soberania da linguagem.
*   **A Regra**: Use `native lua` apenas para:
    1.  Acesso a bibliotecas C externas via FFI.
    2.  Otimizações extremas de loop que ainda não existem no core.
    3.  Interoperabilidade com código legado.
*   **O Contraste**: Código Zenith puro é verificável estaticamente; código Lua nativo é uma "caixa preta".

---

## 6. Checklist de Qualidade

Ao finalizar um módulo, verifique:
- [ ] O código utiliza `Outcome` para operações que podem falhar?
- [ ] As cláusulas `where` protegem as variáveis contra valores inválidos?
- [ ] Os nomes das funções são verbais (`carregar_dados` em vez de `dados`)?
- [ ] A documentação foi movida para um arquivo `.zdoc` para manter o `.zt` limpo?

---
*Zenith Specification v0.3.1 - Manual de Idiomatismos*
