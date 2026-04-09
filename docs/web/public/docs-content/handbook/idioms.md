# Idiomatismo e Boas Práticas (The Zenith Way)

**Objetivo**: Padronizar a escrita de código para máxima clareza e manutenibilidade.  
**Público-alvo**: Desenvolvedores Zenith v0.2.  
**Contexto**: Manual Oficial (v0.2)

## 🎨 Nomenclatura

O Zenith preza pela legibilidade semântica.

- **Variáveis e Funções**: Use `snake_case`.
    - `var player_name = "Zen"`
    - `func calculate_damage()`
- **Tipos (Structs, Traits, Enums)**: Use `PascalCase`.
    - `STRUCT GameManager`
    - `TRAIT PhysicsEntity`
- **Constantes**: Use `SCREAMING_SNAKE_CASE`.
    - `const MAX_LEVEL = 99`

## 🧠 Reatividade: `state` vs `var`

Não torne tudo reativo. A reatividade tem um custo de memória e processamento (metatabelas).

- **Use `state`** apenas para valores que precisam disparar atualizações visuais ou lógicas automáticas em outros lugares (UI, Scores, Stats).
- **Use `var`** para controles internos, contadores de loop e lógica temporária dentro de funções.

## 🧱 Composição sobre Herança

O Zenith não suporta herança de classes. O padrão correto é criar comportamentos pequenos e reutilizáveis com **Traits**.

- Em vez de `Orc herda de Monstro`, use:
    - Trait `Combative` (para atacar).
    - Trait `Vulnerable` (para receber dano).
    - Aplique ambas à Struct `Orc`.

## 🛡️ Tratamento de Erros com `check`

O Zenith evita o uso de `try/catch`. O idiomatismo correto para validações é o uso do `check`.

```text
-- Ruim: Verificação manual poluída
if score < 0:
    error("Inválido")
end

-- Bom: O "Zenith Way"
check score > 0 else "O score não pode ser negativo"
```

## 🧹 Código Limpo e Acessível

- Evite funções gigantescas. Prefira várias funções pequenas e bem nomeadas.
- Documente suas API usando o formato Markdown diretamente nos arquivos `.zt`.
- Use o operador `@` para acessar campos internos em métodos de struct, tornando o código mais conciso.

---

**Status**: Versão 0.2 Completa
