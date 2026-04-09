# Zenith Stdlib Guard Skill

Esta skill define a arquitetura e os padrões de design da biblioteca padrão (stdlib) do Zenith para garantir consistência e ergonomia.

## 🎯 Objetivo
Manter a biblioteca padrão coesa, previsível e fácil de aprender, respeitando os princípios de neurodiversidade da linguagem.

## 🚀 Gatilho (Trigger)
Esta skill deve ser ativada SEMPRE que você:
1. Adicionar uma nova função global (built-in).
2. Criar um novo módulo em `std.*`.
3. Alterar a assinatura de uma função existente da stdlib.

## 📖 Instruções de Execução

### 1. Nomenclatura e Estilo
- **Snake Case:** Use apenas minúsculas e underscores (ex: `read_file`, `append_to`).
- **Verbos Claros:** Use verbos no infinitivo para ações (ex: `clear`, `find`, `match`).
- **Consistência:** Se uma função de lista se chama `length`, use o mesmo nome para strings e grids.

### 2. Tratamento de Erros
- **Bang Operator (!):** Se uma função da stdlib pode falhar suavemente, ela deve suportar o operador `!` ou retornar um tipo explícito que possa ser capturado com `attempt/rescue`.
- **Mensagens Humanas:** Erros internos da stdlib devem ser amigáveis e seguir a Skill de Auditoria de Docs.

### 3. Documentação Mandatória
Ao adicionar uma função à stdlib:
1. Atualize o arquivo `docs/language-spec/stdlib-reference.md` (ou similar).
2. Adicione exemplos de uso `zt` claros.
3. Use blocks de `[!TIP]` para dicas de performance ou uso comum.

### 4. Mapeamento para Lua
- Garanta que a função mapeie para a Lua de forma eficiente.
- Se for uma função nativa da Lua, use um wrapper fino para manter a coerência da sintaxe Zenith.

---

## 🏗️ Exemplo de Padrão
Correto: `print_error(msg: text)`
Incorreto: `PrintError(msg)`, `logError(m)`
