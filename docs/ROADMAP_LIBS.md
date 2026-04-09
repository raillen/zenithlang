# Roadmap de Bibliotecas: Zenith Stdlib v0.6

Este documento define a estratégia, a filosofia e o cronograma de implementação dos módulos oficiais da linguagem Zenith.

## 🎯 Filosofia da Stdlib
1. **Segurança por Padrão**: Operações que podem falhar (IO, Network, Parsing) devem retornar `Outcome<T, E>`.
2. **Zenith-First**: Sempre que possível, implementar a lógica em Zenith. Usar `native lua` apenas para acesso ao sistema operacional ou performance crítica.
3. **API Fluida**: Nomes de funções verbais e verticalidade (legibilidade acima de concisão extrema).
4. **Baterias Inclusas (mas leves)**: Foco nos domínios de Automação, Jogos e UI.

---

## 🚀 Fase 1: Fundação (v0.6.0) - *Em Aberto*
*Foco: Permitir que o Zenith interaja com o mundo real e manipule dados básicos.*

### [ ] `std.fs` (File System)
- **Descrição**: Manipulação de arquivos e pastas no disco.
- **Funcionalidades**: 
    - `fs.read_text_file(path: text) -> Outcome<text, text>`
    - `fs.write_text_file(path: text, content: text) -> Outcome<void, text>`
    - `fs.exists(path: text) -> bool`
    - `fs.create_folder(path: text) -> Outcome<void, text>`
    - `fs.list_folder_content(path: text) -> list<text>`

### [ ] `std.os` (Operating System)
- **Descrição**: Interface com o ambiente e o hardware.
- **Funcionalidades**: 
    - `os.get_env_variable(name: text) -> Optional<text>`
    - `os.get_all_env_variables() -> map<text, text>` (Lista tudo)
    - `os.run_command(cmd: text) -> int`
    - `os.get_system_info() -> SystemInfo` (SO, User, Arq)
    - `os.get_hardware_info() -> HardwareInfo` (RAM, CPU, Bateria)
    - `os.get_special_path(kind: SystemPath) -> Optional<text>`
    - **Variáveis Padrão**: `ZENITH_PROJECT_ROOT`, `ZENITH_VERSION`, etc.

### [ ] `std.json` (Dados)
- **Descrição**: Serialização e desserialização de dados.
- **Funcionalidades**:
    - `parse(json: text) -> Outcome<any, text>`
    - `stringify(value: any) -> text`
    - *Meta*: Mapeamento direto para structs via reflexão básica.

---

## 🌀 Fase 2: Utilidades e Tempo (v0.7.0)
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

## ⚡ Fase 3: Conectividade e Estruturas (v0.8.0+)
*Foco: Web e algoritmos.*

### [ ] `std.collections`
- **Descrição**: Algoritmos sobre listas e mapas.
- **Funcionalidades**: `sort`, `filter`, `map` (lazy), `Set`, `Deque`.

### [ ] `std.http` (Opcional)
- **Descrição**: Cliente HTTP simples e assíncrono.
- **Funcionalidades**: `get(url)`, `post(url, body)`.

---

## ✅ Critérios de Aceite para um Módulo
- [ ] Testes de integração cobrindo casos de sucesso e erro.
- [ ] Documentação em `docs/api/`.
- [ ] Zero vazamento de globais Lua (todo `native lua` deve ser encapsulado).

---
*Última atualização: 09 de Abril de 2026*
