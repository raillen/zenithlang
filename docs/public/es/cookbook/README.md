# Libro de cocina Zenith

> Audiencia: usuario
> Estado: borrador
> Superficie: pública
> Fuente de la verdad: no

## Objetivo

Responda preguntas prácticas:

"¿Cómo hago esto en Zenith?"

El libro de cocina existe para transformar conceptos lingüísticos en soluciones directas.

## Alcance

Incluye recetas para problemas comunes:

- ausencia sin`null`;
- error sin excepción;
- unión segura;
- método abstracto;
- despacho virtual;
- validación de entrada;
- conversión a texto;
- búsqueda segura;
- fallos intencionados;
- pruebas legibles.

No incluye:

- especificación normativa;
- hoja de ruta;
- debate sobre el diseño histórico;
- funciones futuras como si fueran actuales.

## Dependencias

- Aguas arriba:
  -`docs/internal/planning/documentation-roadmap-v1.md`
  - `docs/internal/standards/user-doc-template.md`- Aguas abajo:
  -`docs/public/README.md`
  - `docs/reference/language/`- Código/Pruebas relacionadas:
  -`tests/behavior/MATRIX.md`

## Contenido principal

Formato de receta estándar:

1. Problema.
2. Respuesta corta.
3. Código recomendado.
4. Explicación.
5. Error común.
6. Cuándo no utilizar.

Recetas de inicio recomendadas:

| Receta | Usar cuando |
| --- | --- |
|`absence-without-null.md`| un valor puede no existir |
|`errors-without-exceptions.md`| una operación puede fallar y la persona que llama debe decidir |
|`safe-union-with-enum.md`| un valor puede tener diferentes formatos |
|`abstract-methods-with-trait.md`| varios tipos necesitan cumplir el mismo contrato |
|`virtual-dispatch-with-dyn.md`| valores de diferentes tipos deben ser llamados por el mismo contrato |
|`partial-class-with-apply.md`| desea separar métodos sin fragmentar los datos |
|`callable-delegate.md`| quieres pasar una función como valor |
|`zenith-equivalents-from-other-languages.md`| conoces el concepto en otro idioma y quieres el equivalente Zenith |
|`intentional-failure-builtins.md`| necesitas fallar claramente con`check`, `todo`o`unreachable` |

## Validación

Antes de publicar una receta:

```powershell
python tools/check_docs_paths.py
git diff --check
```

Los ejemplos deben compilarse o marcarse como ilustrativos.

## Historial de actualizaciones

- 2026-04-25: índice inicial del libro de cocina.
- 2026-04-26: se agregó una receta de equivalentes de Zenith para conceptos de otros idiomas.
- 2026-04-26: Se agregó una receta para fallas intencionales con`check`, `todo`y`unreachable`.
