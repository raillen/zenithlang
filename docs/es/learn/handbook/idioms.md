# Idiomatismos y Buenas Prácticas: El Estándar Zenith

Este documento define cómo escribir código Zenith industrial. Seguir estos patrones garantiza que su código sea legible por cualquier otro desarrollador y esté optimizado por el compilador Ascension.

## 1. Nomenclatura e Intención

Zenith utiliza el patrón visual para diferenciar datos de estructuras.

| Elemento | Patrón | Ejemplo |
| :--- | :--- | :--- |
| Variables y Funciones | `snake_case` | `velocidade_actual` |
| Constantes | `SCREAMING_SNAKE` | `GRAVEDAD_MAX` |
| Tipos y Estructuras | `PascalCase` | `PlayerProfile` |

---

## 2. Tipado: Claridad sobre Adivinación

*   **Contexto**: El compilador puede inferir tipos, mas el cerebro humano necesita anclas.
*   **La Regla**: Declare el tipo explícitamente en variables de nivel de módulo o funciones públicas.
*   **Contraste**: A diferencia de JavaScript o Lua, donde el tipo es una sorpresa, en Zenith el tipo es un contrato.

```zt
-- RECOMENDADO
var nivel: int = 1

-- EVITE (en código público)
var nivel = 1
```

---

## 3. Desenvuelto Seguro (Safety First)

Este es el punto más crítico de estabilidad industrial de Zenith.

*   **Contexto**: Manejo de valores nulos o fallos.
*   **La Regla**: Prefiera siempre el operador `or` (valor por defecto) o `match`. Use el operador `!` solo después de una verificación de `is_present()`.
*   **Contraste**: Zenith desalienta la "Programación Orientada al Crash" (usar `!` en todo), promoviendo la resiliencia.

```zt
-- ✅ SEGURO: Código industrial
var usuario = os.get_env("USER") or "admin"

-- ❌ PELIGROSO: Puede causar un Panic en el sistema
var usuario = os.get_env("USER")!
```

---

## 4. Composición sobre Herencia

*   **La Regla**: No intente crear jerarquías de clases. Cree datos (`struct`) y adjunte comportamientos (`trait`).
*   **Ventaja**: Evita el "Problema del Diamante" y hace que el código sea 100% testeable individualmente.

---

## 5. Límites de lo Nativo (Boundary Principle)

*   **Contexto**: Zenith permite usar Lua nativo, mas eso rompe la soberanía del lenguaje.
*   **La Regla**: Use `native lua` solo para:
    1.  Acceso a librerías C externas vía FFI.
    2.  Optimizaciones extremas de bucles que aún no existen en el core.
    3.  Interoperabilidad con código legado.
*   **Contraste**: El código Zenith puro es verificable estáticamente; el código Lua nativo es una "caja negra".

---

## 6. Checklist de Calidad

Al finalizar un módulo, verifique:
- [ ] ¿El código utiliza `Outcome` para operaciones que pueden fallar?
- [ ] ¿Las cláusulas `where` protegen las variables contra valores inválidos?
- [ ] ¿Los nombres de las funciones son verbales (`cargar_datos` en lugar de `datos`)?
- [ ] ¿La documentación fue movida a un archivo `.zdoc` para mantener el `.zt` limpio?

---
*Especificación Zenith v0.3.1 - Manual de Idiomatismos*
