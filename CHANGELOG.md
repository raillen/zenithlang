# Changelog

Todos os marcos do desenvolvimento da linguagem Zenith.

## [0.2.5] - 2026-04-09

### Adicionado (Stdlib Foundation)
- 🖥️ **std.os**: Interface com o sistema operacional, hardware e variáveis de ambiente.
- 📂 **std.fs**: Manipulação de arquivos e pastas (nível atômico e streams).
- 🧭 **std.fs.path**: Submódulo para manipulação inteligente de caminhos.
- 📦 **std.json**: Serialização e desserialização de dados estruturados com suporte a indentação.

### Corrigido (Estabilização do Core)
- 🐛 **Shadowing no Codegen**: Corrigida a declaração dupla de locais que quebrava a reatividade.
- 🏗️ **Aninhamento no Parser**: Corrigido o `parse_block` para suportar estruturas complexas aninhadas (match/while/if).
- 🔤 **Lexer Escapes**: Adicionado suporte a escapes de aspas e chaves (`\{`, `\}`) em strings literais.
- 🛠️ **Operadores no Codegen**: Adicionado suporte para operador de tamanho `#`, listas `[]` e mapas `{}`.

## [0.2.0] - 2026-04-08

### Adicionado (Finalização do Core)
- 🏗️ **Lowering & IR**: Fase de transformação intermediária para desaçucaramento e otimização.
- ⚡ **Constant Folding**: Otimização automática de expressões constantes em tempo de compilação.
- 🧩 **Destruturação Completa**: Suporte para desmontar listas e structs em `match` e declarações `var`.
- 🧬 **Genéricos com Restrições**: Introdução de `where T is Trait` para segurança em tipos genéricos.
- 🛡️ **Traits Avançadas**: Validação de satisfatibilidade e Implementações Padrão (Default Impls).
- 🚦 **Operador `where`**: Contratos de dados em runtime validados automaticamente em structs.
- 📦 **Sistema de Projeto**: Lançamento do formato `.ztproj` para orquestração de builds.
- 🎹 **Prelude Automática**: Carregamento de `std.core` sem necessidade de import explícito.
- 🔪 **Slicing e Spread**: Manipulação poderosa de coleções com operadores `..` e `[start..end]`.
- 🏷️ **Keywords Type e Union**: Distinção clara entre apelidos semânticos e tipos soma.

### Alterado
- 🍭 **Açúcar de Sintaxe**: Substituído `.campo` por `@campo` para acesso ao `self`.
- 🔗 **Chaining Vertical**: O ponto `.` agora é usado exclusivamente para encadeamento de funções (UFCS).
- 💎 **Tipagem Obrigatória**: Declarações de variáveis agora exigem tipo explícito (`var x: int`).
- ➡️ **Sintaxe de Retorno**: Padronização do uso de `->` para tipos de retorno em funções.

### Corrigido
- 🐛 Correção de bugs críticos no Binder relacionados a dependências circulares.
- 🐛 Estabilização do Lexer para lidar com interpolações complexas e caracteres especiais.

## [0.1.0] - 2026-04-07
- Versão inicial com suporte básico a variáveis, funções e structs.
- Transpilação básica para Lua.
