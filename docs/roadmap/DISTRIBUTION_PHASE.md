# Roadmap: Zenith Distribution & Professionalization (Phase 4)

Este roadmap define a transição do Zenith de um sistema baseado em scripts para uma infraestrutura de linguagem profissional e independente, focada no **Modo A (Launcher Nativo em C)**.

## 🎯 Objetivos Principais
1. **Opaquicidade**: Esconder a implementação em Lua do usuário final.
2. **Portabilidade**: Criar um executável único (`zt.exe` / `zt`) que contenha o compilador.
3. **Ergonomia**: Eliminar a necessidade de configurar `LUA_PATH` ou variáveis de ambiente manualmente.
4. **Resiliência**: Garantir que a `stdlib` seja encontrada independentemente de onde o comando for chamado.

---

## 🛠️ Etapa 1: O Launcher Nativo (The Seed)
Transformar o `zt.bat` em um binário real escrito em C que incorpora a VM Lua.

- [ ] **Design do Launcher (C)**: Criar o `src/cli/launcher.c`.
    - Integrar a API da Lua (`lua_State`).
    - Embutir o `dist/ztc.lua` como um recurso estático (string binária).
    - Implementar a lógica de descoberta de caminhos (`GetModuleFileName` no Windows / `readlink` no Linux).
- [ ] **Sistema de Build Nativo**: Criar um `Makefile` ou script de compilação para o launcher.
    - Linkagem estática com `liblua.a` para zero dependências externas.
- [ ] **Primeiro Executável**: Gerar o `zt.exe` inicial.

## 📦 Etapa 2: Estrutura de Instalação (The Colony)
Definir como o Zenith se organiza no disco após ser instalado.

- [ ] **Layout de Pastas (Standard)**:
    - `/bin/zt.exe` (O Launcher)
    - `/lib/std/` (Arquivos `.zt` da biblioteca padrão)
    - `/include/` (Headers nativos, se houver)
- [ ] **Discovery Protocol**:
    - O launcher deve detectar automaticamente onde está a sua `/lib/std/` relativa ao executável.
    - Injetar esses caminhos no `package.path` da VM Lua internamente.

## 🚀 Etapa 3: Purificação do Workflow (The Sovereign)
Remover os andaimes (scaffolding) do desenvolvimento.

- [ ] **Modo Standalone no Compilador**:
    - Atualizar o `emitter` para opcionalmente injetar o `zenith_rt.lua` no topo de cada arquivo gerado.
    - Permitir que programas Zenith rodem apenas com `lua.exe`, sem precisar da pasta `src/` por perto.
- [ ] **ZPM (Zenith Package Manager)**:
    - Finalizar o `src/cli/zpm.zt` para gerenciar a pasta `modules/` local.
    - O compilador deve priorizar `./modules/` -> `Installation/lib/std/`.

## 💎 Etapa 4: UX & Polimento
- [ ] **Instalador Automatizado**: Script que compila o launcher, move a stdlib e adiciona ao PATH.
- [ ] **Verificação de Versão**: `zt --version` deve reportar a versão do Ascension e do Runtime.

---

## 📊 Matriz de Esforço vs. Impacto

| Tarefa | Esforço | Impacto | Status |
| :--- | :--- | :--- | :--- |
| Launcher em C | Médio | Altíssimo (DX) | ⏳ Planejado |
| Injeção de StdLib | Baixo | Médio (Robustez) | ⏳ Planejado |
| Bundle Standalone | Médio | Alto (Distribuição) | ⏳ Planejado |
| Script de Instalação | Baixo | Alto (Adoção) | ⏳ Planejado |

---
*Zenith: A soberania técnica através da simplicidade arquitetural.*
