# Roadmap de Consolidación: Zenith Core

Este documento sigue la consolidación técnica del núcleo del lenguaje Zenith y registra el camino que llevó hacia Ascension Final.

## 1. Visión General

Mantengo Zenith como un lenguaje enfocado en la accesibilidad cognitiva, claridad sintáctica y madurez técnica, sin renunciar al rendimiento ni a una lectura hospitalaria.

## 2. Fases 1 a 3: El Corazón (v0.2.5) - Concluido

Enfoque: establecer la gramática, el sistema de tipos y la infraestructura básica.

- [X] ADTs y `match`
- [X] sistema de tipos con `trait`, genéricos y `where`
- [X] async/await integrado
- [X] interoperabilidad con `native lua`
- [X] lambdas, spread, slicing y ergonomía de superficie

## 3. Fase 4: Herramientas y Fundación (v0.2.5) - Concluido

Enfoque: permitir que Zenith interactuara con el mundo real.

- [X] CLI estable con soporte para build y run
- [X] sistema de proyectos vía `.ztproj`
- [X] bibliotecas base como `std.os`, `std.fs` y `std.json`

## 4. Fase 5: Utilidades y Ecosistema (v0.2.6 - v0.2.7) - Concluido

Enfoque: expandir las herramientas para uso práctico.

- [X] `std.time` y `std.text`
- [X] ZPM para dependencias externas
- [X] manual interactivo y recursos de apoyo

## 5. Fase 6: Blindaje y Calidad (v0.2.8 - v0.2.9) - Concluido

Enfoque: blindar la plataforma antes del self-hosting.

- [X] ZTest integrado al CLI
- [X] informes visuales y exportación de datos
- [X] ejecución paralela y aislamiento de suites
- [X] snapshot testing y ampliación de cobertura global

## 6. Fase 7: Zenith Ascension (v0.3.0 - v0.3.5) - En Abierto

Enfoque: auto-hospedaje, optimización y soberanía total del pipeline.

- [X] **v0.3.0 (Soberano)**: Motor Ascension Finalizado (FFI, Multi-target, Bundle).
- [X] **v0.3.1 (ZDoc)**: Soporte a archivos .zdoc y Pureza CLI consolidada.
- [ ] **v0.3.2 (Lexer)**: Analizador léxico reescrito en Zenith puro.
- [ ] **v0.3.3 (Parser)**: Algoritmo Pratt Parser portado a Zenith.
- [ ] **v0.3.4 (Binder)**: Tabla de símbolos y resolución semántica nativa.
- [ ] **v0.3.5 (Emitter)**: Generador de código (Codegen) operando en Zenith.

---
## 7. Estudios que continúan después de Ascension

Estos ítems registran la transición al ecosistema Zenith v0.3.1:

- [X] **Zenith Doc-Linking (ZDoc)**: Soporte a archivos `.zdoc` externos para documentación desacoplada, utilizando la etiqueta `@target` para asociar prosa a símbolos sin poluir el código fuente.
- [X] **Módulo Regex Nativo**: Implementación del Fluent Builder para la construcción de patrones rítmicos y legíveis.
- [X] **Manual Interactivo (ZMan)**: Refactorización a Zenith puro, integrando el sistema de Regex para colorización sin dependencia de bloques nativos.
- [X] **Ampliación de la Pureza CLI**: Reducción drástica de `native lua` en las herramientas centrales. ZPM, ZTest y ZMan ahora están orquestados en Zenith soberano.

## 8. Próximos Passos (Trascendencia v1.0)

Con el motor Ascension Finalizado y la CLI Purificada, el enfoque se desplaza hacia la fundación visual e industrial:

- [ ] **Zenith Intersect (UI)**: Framework declarativo y reactivo.
- [ ] **Borealis (Juegos)**: Motor de juegos industrial sobre Raylib.
- [ ] **Atlas (Datos)**: ORM de tipo seguro para SQLite/Postgres.

*Actualizado el: 11-04-2026 (Zenith v0.3.1)*
