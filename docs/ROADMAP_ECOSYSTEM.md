# Roadmap de Ecossistema: Frameworks Zenith

Este documento define a visão, as diretrizes técnicas e o cronograma de desenvolvimento dos frameworks oficiais que orbitam a linguagem Zenith.

## 🎯 Diretrizes do Ecossistema
1.  **MVVM por Padrão**: Toda interação de UI deve ser baseada em estado reativo (`state` e `computed`).
2.  **Abstração Sem Perda**: Frameworks devem esconder a complexidade (ex: C Bindings), mas permitir acesso ao `native lua` se necessário.
3.  **Consistência Aura**: Todos os frameworks devem seguir a identidade visual e os princípios de design da marca Zenith.
4.  **Zero-Setup**: Um desenvolvedor deve conseguir iniciar um projeto `ZenAura` ou `ZenApex` via `zpm init` sem configurar nada manualmente.

---

## 🚀 Fase 5: Persistência e Infra (v0.2.9)
*Foco: Garantir que o Zenith consiga "lembrar" de dados e falar com bibliotecas externas.*

### [ ] `ZenDB` (Database Framework)
- **Necessidade**: O Zenith precisa de armazenamento local robusto para games e ferramentas desktop.
- **Implementação**:
    - [ ] **Driver SQLite Nativo**: Binding FFI para SQLite.
    - [ ] **Zenith ORM (Opcional)**: Mapear `structs` diretamente para tabelas.
    - [ ] **API Fluida**: `db.table("users").where(u => u.age > 18).fetch()`.
- **Evolução**: v0.2.9 (Alpha) -> v0.2.10 (Migrations).

### [ ] `std.ffi` (Language Infrastructure)
- **Necessidade**: Chamar funções C sem precisar escrever código em C.
- **Implementação**:
    - [ ] Sintaxe `native "lib" func name(...)`.
    - [ ] Carregamento dinâmico de bibliotecas em tempo de execução.

---

## 🔱 Fase 6: Zenith Ascension (v0.3.0)
*Foco: Independência total e maturidade técnica.*

### [ ] Self-Hosted Compiler
- **Missão**: Reescrever o compilador atual (Lua) inteiramente em Zenith.
- **Bootstrapping**: Garantir estabilidade total do ecossistema.

---

## 🌀 Fase 7: A Experiência Visual (v0.4.0)
*Foco: Transformar o Zenith na melhor linguagem para criar interfaces bonitas e rápidas.*

### [ ] `ZenAura` (UI Framework)
- **Necessidade**: Substituir o desenvolvimento de UI complexo por algo declarativo e premiun.
- **Diretrizes**: Reatividade total (MVVM) e componentes reutilizáveis.
- **Implementação**:
    - [ ] **Virtual DOM/Node Tree**: Gerenciamento eficiente de elementos.
    - [ ] **Backend Flexível**: Renderizar em HTML (Web) ou Raylib (Desktop).
    - [ ] **Aura Tokens**: Sistema de cores e espaçamento integrado.

---

## 🛠️ Fase 8: O Ecossistema Profissional (v0.5.0)
*Foco: Dar ao desenvolvedor ferramentas de nível industrial.*

### [ ] `Zenith LSP` (Tooling)
- **Necessidade**: Autocompletar, "Go to Definition" e erro em tempo real na IDE e VS Code.
- **Implementação**:
    - [ ] Implementar o servidor de linguagem em Zenith (Self-testing).
    - [ ] Integração com o sistema de tipos semanticamente correto.

---

## 🏹 Fase 9: O Ápice e Conectividade (v0.6.0)
*Foco: Games de alta performance e sistemas distribuídos.*

### [ ] `ZenApex` (Game Engine)
- **Necessidade**: Um motor de jogos que pareça moderno e seja ultra-produtivo.
- **Implementação**:
    - [ ] **Scene Graph**: Hierarquia de objetos espaciais.
    - [ ] **Input Unified**: Teclado, Mouse e Gamepad abstraídos.
    - [ ] **Visual Scripting Integration**: Fazer o parser ler fluxos visuais.

### [ ] `ZenEther` (Web Framework)
- **Necessidade**: Tornar o Zenith a escolha certa para backends rápidos e seguros.
- **Implementação**:
    - [ ] **Zero-Config Routing**: Rotas baseadas no sistema de arquivos.
    - [ ] **Middleware reativo**: Filtros de autenticação e logs.

---

## 🏁 Versão 1.0.0: Zenith Horizon
*O marco de estabilidade e produção em larga escala.*

---

## 📈 Histórico de Versões e Progresso
- **v0.2.x (Atual)**: Consolidação da Stdlib e ZPM.
- **v0.3.0 (Próximo)**: Zenith Ascension (Self-Hosting Compiler).
- **v0.4.x+**: Era dos Frameworks (ZenAura, ZenApex).

---
*Última atualização: 10 de Abril de 2026*
