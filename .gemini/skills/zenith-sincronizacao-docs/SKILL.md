# Zenith Doc Sync Skill

Esta skill descreve o processo obrigatório para manter a documentação da linguagem Zenith (zt) em sincronia com o código-fonte do compilador.

## 🎯 Objetivo
Garantir que qualquer mudança em palavras-chave (keywords), tipos primitivos, estruturas de controle ou funções da biblioteca padrão (stdlib) seja refletida imediata e consistentemente nos arquivos de documentação em `/docs`.

## 🚀 Gatilho (Trigger)
Esta skill deve ser ativada SEMPRE que você:
1. Adicionar, remover ou renomear um `TokenKind` em `src/syntax/lexer/lexer.lua`.
2. Alterar a gramática no `parser.lua` (ex: mudar o fechamento de um bloco de `end` para outro terminador).
3. Modificar o comportamento de tipos primitivos ou o sistema de tipos.
4. Adicionar novas funções globais ou módulos no compilador/stdlib.

## 📖 Instruções de Execução

### 1. Identificação da Mudança
Liste os termos técnicos que foram alterados. Por exemplo:
- Mudança: Troca de `type` para `struct`.
- Novo Recurso: Adição de `repeat X times`.

### 2. Auditoria de Documentos
Execute uma busca global (grep) no diretório `/docs` e no `README.md` pelos termos afetados.
- Arquivos prioritários: `docs/language-spec/syntax.md`, `README.md`.

### 3. Atualização de Conteúdo
Atualize a documentação seguindo as diretrizes de design do Zenith:
- **Linguagem Acessível:** Mantenha o tom amigável, focado em TDAH/Dislexia.
- **Emojis:** Use emojis como âncoras visuais para seções.
- **Alertas:** Use blocks de `[!TIP]`, `[!IMPORTANT]` ou `[!WARNING]` conforme necessário.
- **Exemplos de Código:** Atualize TODOS os blocos de código `zt` para usar a nova sintaxe.

### 4. Validação Tooling
Execute a ferramenta de validação para garantir que não restaram inconsistências:
```bash
lua tools/check_docs_sync.lua
```

### 5. Registro no Changelog
Adicione uma entrada no `CHANGELOG.md` na seção correspondente (`Added`, `Changed`, `Fixed`, `Removed`) sob a versão atual ou `[Unreleased]`.

---

## 💡 Exemplo de Aplicação
Se você alterar a forma como funções são declaradas:
1. Busque `func` em `/docs`.
2. Atualize a seção `150: ### 🏗️ Criando Funções` em `syntax.md`.
3. Verifique se o script `check_docs_sync.lua` reporta algum erro.
4. Adicione ao Changelog: `### Alterado: Sintaxe de funções agora usa -> para retorno.`
