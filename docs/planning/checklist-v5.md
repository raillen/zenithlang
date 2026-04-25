# Zenith Product & Ecosystem Checklist (v5.0)

> Checklist operacional para ferramentas, frameworks e experiência do desenvolvedor.
> Derivado de `docs/planning/roadmap-v5.md`.

---

## 1. ZPM & Registry (Foco Atual)

### [R5.M2] - Evolução do ZPM
- [x] Binário `zpm.exe` independente.
- [x] Registro externo `index.ztproj` via HTTP.
- [x] Comando `update-registry`.
- [x] Implementação de SemVer (`^`, `~`) para Tags de Git.
- [x] Bridge `zt pkg` integrada ao compilador principal.
- [ ] Implementação de dependências transitivas completas (resolução de conflitos).

---

## 2. Editor & IDE (DX)

### [R5.M1] - Compass LSP & Marketplace
- [x] Diagnostics e Hover em tempo real.
- [x] Autocomplete contextual de membros e imports.
- [ ] Preparação do pacote `.vsix` para publicação.
- [ ] Snippets para `fn`, `struct`, `match` e `import`.
- [ ] Template `web-api` básico integrado ao `zpm init`.

---

## 3. Web & Frameworks

### [R5.M3] - Zenith Web Lite
- [ ] Definir interface de `RouteHandler`.
- [ ] Implementar extrator de Path Params (ex: `/user/:id`).
- [ ] Criar middleware de Logger padrão.
- [ ] Criar exemplo de Todo List API funcional.

---

## 4. Bindings & Interop

### [R5.M4] - C-Binding Pack
- [ ] Criar `sqlite_zt` (módulo wrapper).
- [ ] Criar `http_zt` (módulo wrapper para libcurl).
- [ ] Validar gerenciamento de memória (Manual x ARC) nos bindings.

---

## 5. Visual Tools (Borealis)

### [R5.M6] - Borealis Integration
- [ ] Criar link de "live preview" entre código Zenith e Viewport do Borealis.
- [ ] Sincronização de definições de cores e estilos.

---

## Regras de Qualidade (Produto)
- [ ] Todo comando CLI deve responder em < 200ms (percepção de velocidade).
- [ ] Todo erro de usuário nas ferramentas deve sugerir um "fix" ou link de doc.
- [ ] Documentação de "Getting Started" deve ser validada por um usuário novo periodicamente.
