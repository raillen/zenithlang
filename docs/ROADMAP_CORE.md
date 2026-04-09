# Roadmap de Consolidação: Zenith Core v1.0

Este documento detalha o caminho técnico para finalizar o núcleo da linguagem Zenith, partindo da versão atual (v0.2.0) até o lançamento da v1.0.0 estável.

## 🎯 Visão Geral
O objetivo é transformar o Zenith em uma linguagem de produção, focada em acessibilidade cognitiva (TDAH/Dislexia) e alta performance sobre a VM Lua.

---

## 🛠️ Fase 1: Ergonomia e Expressividade (v0.3.0)
*Foco: Reduzir a verbosidade e aumentar o prazer de escrever Zenith.*

- [ ] **Funções Anônimas (Lambdas)**: Sintaxe `(a, b) => expr` para callbacks rápidos.
- [ ] **Destruturação em Parâmetros**: `func mover(Player { @posicao })` para acesso direto a campos na assinatura.
- [ ] **Spread Operator em Expressões**: Espalhar coleções `[..lista, item]` e mapas `{..config, cor: "azul"}`.
- [ ] **Type Aliases Avançados**: Suporte total a `type` para apelidos semânticos e `union` para tipos soma complexos.

## 🌀 Fase 2: Sistema de Tipos e ADTs (v0.4.0)
*Foco: Segurança de dados e modelagem complexa.*

- [ ] **Enums com Dados (Sum Types)**: `enum Status: Sucesso(valor: T), Erro(msg: text)`.
- [ ] **Generic Constraints Profundas**: Validação de métodos de Traits dentro de contextos genéricos.
- [ ] **Slicing de Coleções**: Suporte nativo para fatiamento `lista[0..5]` em todos os tipos de coletores.
- [ ] **Option/Result Types**: Tipos nativos para lidar com ausência de valor e erros sem exceções.

## ⚡ Fase 3: Infraestrutura e Performance (v0.5.0)
*Foco: Otimização e integração com a plataforma Lua.*

- [ ] **Deep Desugaring na IR**: Mover toda a lógica de `match` e `where` para o Lowerer, limpando o Codegen.
- [ ] **Transparent Async/Await**: Implementação robusta de corrotinas Zenith que não bloqueiam a execução.
- [ ] **Blocos Native**: Keyword `native lua ... end` para integração direta e sem overhead com o ecossistema Lua.
- [ ] **Trait `Iterable`**: Tornar o loop `for-in` extensível para qualquer estrutura de dados do usuário.

## 📦 Fase 4: Tooling e Consolidação (v1.0.0)
*Foco: Ecossistema e estabilidade final.*

- [ ] **Zenith Project System (ZPM)**: Gerenciamento de dependências externas via `.ztproj`.
- [ ] **Standard Library (Stdlib)**: Finalização dos módulos `std.io`, `std.os`, `std.math` e `std.json`.
- [ ] **Self-Hosting (Opcional)**: Reescrever partes do compilador em Zenith para provar a maturidade da linguagem.
- [ ] **Auditoria de Estabilidade**: Suite de testes com +500 casos de uso cobrindo edge cases.

---

## 📖 Diretrizes de Design (Core)
1. **Nenhuma Inferência de Tipo Global**: O Zenith exige clareza. `var x: int` é a lei.
2. **Verticalidade**: Priorizar sintaxes que permitam leitura de cima para baixo.
3. **Explícito é melhor que Implícito**: Evitar comportamentos "mágicos" que confundam o programador.
4. **Erros Amigáveis**: O compilador deve ser um mentor, não um juiz. Mensagens de erro devem sugerir soluções.

---
*Última atualização: 08 de Abril de 2026*
