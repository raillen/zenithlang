# 🌌 Manifesto do Ecossistema Zenith

Este documento define a visão técnica e filosófica para a expansão do Zenith além de seu núcleo. O ecossistema Zenith é construído sobre três pilares: **Simplicidade**, **Reatividade** e **Poder Nativo**.

## 🎯 A Trindade Técnica

1.  **Binding via FFI (Poder Nativo)**: O Zenith não competirá com bibliotecas C maduras; ele as abraçará. Usaremos FFI para trazer o poder de motores gráficos e bancos de dados sem o custo de abstrações pesadas.
2.  **Arquitetura MVVM (Reatividade)**: O estado é a fonte da verdade. O Zenith usará seu sistema nativo de `zt.state` para criar interfaces que respondem instantaneamente a mudanças de dados.
3.  **Outcome-First (Segurança)**: Nenhum framework do ecossistema deve lançar erros inesperados; falhas são cidadãs de primeira classe através do tipo `Outcome`.

---

## 🏛️ Os Frameworks Oficiais

### 1. 🎨 ZenAura (UI & Desktop)
*   **Missão**: Criar interfaces premium e declarativas.
*   **Diferencial**: Estilização baseada em tokens (Aura Design System) e reatividade total.

### 2. 🎮 ZenApex (Game Engine)
*   **Missão**: Desenvolvimento de jogos focado em produtividade.
*   **Diferencial**: Binding ultra-rápido com Raylib/LuaJIT e uma arquitetura ECS (Entity Component System) simplificada e opcional.

### 3. 🌐 ZenEther (Web & Connectivity)
*   **Missão**: O "Ether" que conecta aplicações.
*   **Diferencial**: Servidores assíncronos baseados em corrotinas que parecem código síncrono.

### 4. 🗄️ ZenDB (Data Mastery)
*   **Missão**: Persistência transparente.
*   **Diferencial**: SQL com Type-Mapping automático para structs Zenith.

---

## 🗺️ Roadmap de Maturidade (2026)

### Fase 1: A Ponte (Maturidade da Linguagem)
- [ ] **Implementação do Módulo FFI**: Adicionar suporte nativo no compilador para carregar libs externas (`.dll`/`.so`) sem código C manual.
- [ ] **Zenith LSP**: Desenvolver o Language Server para autocompletar e diagnósticos reais.

### Fase 2: A Base (ZenDB)
- [ ] Lançar a primeira versão do **ZenDB** focada em SQLite.
- [ ] Sistema de Migrations simplificado em Zenith.

### Fase 3: A Aura (UI)
- [ ] Draft do **ZenAura** DSL (Domain Specific Language) para componentes.
- [ ] Integração reativa com um backend de janelas (Tauri ou Raylib).

### Fase 4: O Ápice (Games)
- [ ] Prototipagem do **ZenApex** em cima de bibliotecas FFI.
- [ ] Sistema de Sprites, Input e Som.

---

## 📜 Declaração de Intenção
O Zenith nasceu para simplificar. O ecossistema Zenith existe para dar essa simplicidade o poder de criar o impossível.
