# Roadmap de Consolidação: Zenith Core

Este documento detalha o caminho técnico do núcleo da linguagem Zenith, refletindo o progresso real do compilador e as metas de evolução granular.

## 🎯 Visão Geral
O Zenith é uma linguagem focada em acessibilidade cognitiva (TDAH/Dislexia) e alta performance sobre a VM Lua, priorizando clareza sintática e segurança de dados.

---

## ✅ Fase 1, 2 e 3: O Coração (v0.2.5) - *Concluído*
*Foco: Estabelecer a gramática, o sistema de tipos e a infraestrutura básica.*

- [x] **ADTs e Match**: Suporte a Enums genéricos e destruturação poderosa.
- [x] **Sistema de Tipos**: Verificação estática, Traits e restrições genéricas (`where`).
- [x] **Concorrência**: Async/Await integrado via corrotinas transparentes.
- [x] **Interoperabilidade**: Blocos `native lua` para acesso direto ao ecossistema Lua.
- [x] **Ergonomia**: Lambdas, Spread, Slicing e Operador `?` (Optional/Outcome).

---

## 🚀 Fase 4: Tooling e Fundação (v0.2.5) - *Concluído*
*Foco: Permitir que o Zenith interaja com o mundo real.*

- [x] **Transpiler CLI**: Orquestrador `ztc` estável com suporte a `--run`.
- [x] **Project System**: Gerenciamento de build via arquivos `.ztproj`.
- [x] **Bibliotecas Base**: Implementação de `std.os`, `std.fs`, `std.json`.

---

## 🌀 Fase 5: Utilidades e Ecossistema (v0.3.0) - *Em Aberto*
*Foco: Expandir as ferramentas para o desenvolvedor.*

- [ ] **Bibliotecas de Suporte**: `std.time` (delays/clocks) e `std.text` (manipulação de strings).
- [ ] **ZPM (Git/HTTP)**: Suporte para baixar e gerenciar dependências externas.
- [ ] **Orquestrador de Testes**: Melhoria no `test_all.lua` para relatórios visuais.

---

## 🛠️ Fase 6: Profissionalização (v0.4.0+)
*Foco: Estabilidade e ferramentas de IDE.*

- [ ] **LSP Básico**: Autocomplete e diagnósticos em tempo real para editores.
- [ ] **Reflexão Básica**: Capacidade de inspecionar campos de structs em runtime (necessário para JSON automático).
- [ ] **Auditoria 1.0**: Expansão da suite para +500 testes integrados.

---

## 📖 Diretrizes de Design
1. **Verticalidade**: Priorizar sintaxes que permitam leitura de cima para baixo.
2. **Explícito é melhor que Implícito**: Evitar comportamentos "mágicos" que confundam o programador.
3. **Erros Amigáveis**: O compilador deve sugerir soluções (Mentoria).

---
*Atualizado em: 09 de Abril de 2026 (v0.2.5)*
