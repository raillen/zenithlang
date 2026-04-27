# Zenith Product & Ecosystem Roadmap (v5.0)

## Objetivo

Focar na entrega de valor, ferramentas e frameworks que tornam o Zenith utilizável comercialmente. Este roadmap é independente do desenvolvimento do motor da linguagem (v4).

## Pilares do Produto

- **DX (Developer Experience)**: Tooling de editor e feedback em tempo real.
- **Distribuição**: Gerenciamento de pacotes e registro oficial.
- **Web & Backend**: Frameworks e bibliotecas para APIs.
- **Integração C**: Bindings de alto nível para bibliotecas populares.
- **Aprendizado**: Disponibilização de ambiente para testes rápidos (Playground).

---

## Fases do Roadmap 5.0

### R5.M1 - Tooling: Compass LSP & VSCode
Foco em produtividade e adoção rápida.
- [ ] Publicação da Extensão no VSCode Marketplace.
- [ ] Templates de projeto (`zpm init --template web|app`).
- [ ] Snippets de código para estruturas comuns.
- [ ] Integração de comandos de Build/Run direto no editor.

### R5.M2 - Distribuição: ZPM Registry
Tornar o Zenith uma linguagem conectada.
- [x] ZPM standalone com comandos de gerenciamento.
- [x] Registro externo sincronizado via HTTP (GitHub-based).
- [x] Resolução de versões SemVer via Git Tags.
- [ ] Interface Web para busca de pacotes (Registry UI - futura).

### R5.M3 - Backend: Zenith Web Lite
O "Sinatra" do Zenith para APIs REST rápidas.
- [ ] Implementação de roteador performático.
- [ ] Middlewares de serialização JSON.
- [ ] Tratamento global de exceções/erros.
- [ ] Guia "Hello World to API" em 15 minutos.

### R5.M4 - Ecossistema: C-Binding Pack
Conectar o Zenith com o mundo MSVC/GCC.
- [ ] Binding oficial `sqlite3` (persistência).
- [ ] Binding oficial `libcurl` (networking).
- [ ] Utilitários de mapeamento automático C -> Zenith.

### R5.M5 - Learning: Playground & Trilha
Reduzir a barreira de instalação.
- [ ] Playground Web (Compilação WASM ou Backend Exec).
- [ ] Série de exemplos curados para "quick start".
- [ ] Trilha de aprendizado "Zero to Hero".

### R5.M7 - Script Mode & REPL
Reduzir atrito para experimentação e aprendizado.
- [x] `zt run arquivo.zt` — executar arquivo avulso sem projeto (`zenith.ztproj`).
- [ ] `zt repl` — REPL interativo com interpretador ZIR/HIR.
  - Interpretar ZIR em memória sem emitir C nem invocar gcc.
  - Estado persistente entre linhas (variáveis, funções definidas).
  - Acesso à stdlib via runtime interpretado.
  - Estimativa: ~3000-5000 LOC, 3-4 semanas.
  - Depende de: avaliador de ZIR completo (aritmética, chamadas, ARC, coleções).

### R5.M6 - Interface: Borealis Studio (POC)
Exploração de ferramentas visuais.
- [ ] POC de integração entre UI Zenith e o Studio.
- [ ] Visualização de árvores de componentes.
- [ ] Edição visual de propriedades de UI.

---

## KPI de Produto (v5)

- **Adoção**: Extensão VSCode com > 100 installs (meta inicial).
- **Utilidade**: Existência de pelo menos 5 pacotes no registro oficial.
- **Performance**: Tempo de setup de projeto novo < 30 segundos.
