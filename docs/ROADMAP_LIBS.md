# Roadmap de Bibliotecas: Zenith Stdlib

Este documento define a estratégia, a filosofia e o cronograma de implementação dos módulos oficiais da linguagem Zenith.

## 🎯 Filosofia da Stdlib
1. **Segurança por Padrão**: Operações que podem falhar (IO, Network, Parsing) devem retornar `Outcome<T, E>`.
2. **Zenith-First**: Sempre que possível, implementar a lógica em Zenith. Usar `native lua` apenas para acesso ao sistema operacional ou performance crítica.
3. **API Fluida**: Nomes de funções verbais e verticalidade (legibilidade acima de concisão extrema).
4. **Baterias Inclusas (mas leves)**: Foco nos domínios de Automação, Jogos e UI.

---

## 🚀 Fase 1: Fundação (v0.2.5) - *Concluído*
*Foco: Permitir que o Zenith interaja com o mundo real e manipule dados básicos.*

### [x] `std.fs` (File System)
- **Descrição**: Manipulação de arquivos e pastas no disco.
- **Nível Atômico**: 
    - [x] `fs.read_text_file(path: text) -> Outcome<text, text>`
    - [x] `fs.write_text_file(path: text, content: text) -> Outcome<void, text>`
    - [x] `fs.append_text(path: text, content: text) -> Outcome<void, text>`
    - [x] `fs.prepend_text(path: text, content: text) -> Outcome<void, text>`
    - [x] `fs.exists(path: text) -> bool`
    - [x] `fs.remove_file(path: text) -> Outcome<void, text>`
    - [x] `fs.remove_folder(path: text, recursive: bool) -> Outcome<void, text>`
- **Nível de Fluxo (Streams)**:
    - [x] `fs.open_file(path: text, mode: FileMode) -> Outcome<FileHandle, text>`
    - [x] `struct FileHandle`: `read_line()`, `write(text)`, `close()`.
- **Submódulo `std.fs.path`**:
    - [x] `path.join(parts: list<text>) -> text`
    - [x] `path.extension(p: text) -> text`
    - [x] `path.basename(p: text) -> text`
    - [x] `path.dirname(p: text) -> text`
    - [x] `path.is_absolute(p: text) -> bool`

### [x] `std.os` (Operating System)
- **Descrição**: Interface com o ambiente e o hardware.
- **Funcionalidades**: 
    - [x] `os.get_env_variable(name: text) -> Optional<text>`
    - [x] `os.get_all_env_variables() -> map<text, text>`
    - [x] `os.run_command(cmd: text) -> int`
    - [x] `os.get_system_info() -> SystemInfo`
    - [x] `os.get_hardware_info() -> HardwareInfo`
    - [x] `os.get_special_path(kind: SpecialPath) -> text`
    - **Variáveis Padrão**: `version`, `platform`, `arch`, `project_root`.

### [x] `std.json` (Dados estruturados)
- **Descrição**: Serialização e desserialização de dados estruturados.
- **Funcionalidades**:
    - [x] `json.parse(json: text) -> Outcome<any, text>`
    - [x] `json.decode<T>(json: text) -> Outcome<T, text>` (Mapeamento para Structs)
    - [x] `json.stringify(value: any, indent: int) -> text` (indent: 0=min, 1-8=spaces, -1=tabs)
    - [x] `json.minify(json: text) -> Outcome<text, text>`
    - [x] `json.beautify(json: text, indent: int) -> Outcome<text, text>`
- **Atalhos de Arquivo**:
    - [x] `json.read_file(path: text) -> Outcome<any, text>`
    - [x] `json.write_file(path: text, value: any, indent: int) -> Outcome<void, text>`

---

## 🌀 Fase 2: Utilidades e Tempo (v0.3.0)
*Foco: Controle de fluxo temporal e manipulação avançada de texto.*

### [ ] `std.time`
- **Descrição**: Gestão de tempo e delays.
- **Funcionalidades**:
    - `now() -> float` (Timestamp)
    - `sleep(ms: int)` (Integrado ao `async`)
    - `struct Duration` (Abstração de intervalos)

### [ ] `std.text`
- **Descrição**: Além do que o core oferece para strings.
- **Funcionalidades**:
    - `split(target: text, sep: text) -> list<text>`
    - `join(parts: list<text>, sep: text) -> text`
    - `replace_all(target: text, pattern: text, replacement: text) -> text`

---

## 🛠️ Fase 3: Tooling e Ecossistema (v0.4.0)
*Foco: Gestão de pacotes e experiência do desenvolvedor.*

### [ ] ZPM (Zenith Package Manager)
- Suporte a dependências via Git/HTTP.
- Publicação de módulos oficiais.

---

## ✅ Critérios de Aceite para um Módulo
- [ ] Testes de integração cobrindo casos de sucesso e erro.
- [ ] Documentação em `docs/api/`.
- [ ] Zero vazamento de globais Lua (todo `native lua` deve ser encapsulado).

---
*Última atualização: 09 de Abril de 2026 (v0.2.5)*
