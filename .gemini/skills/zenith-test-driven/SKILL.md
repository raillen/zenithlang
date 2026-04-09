# Zenith Test-Driven Evolution Skill

Esta skill descreve o workflow obrigatório de Test-Driven Development (TDD) para garantir a estabilidade do compilador Zenith.

## 🎯 Objetivo
Prevenir regressões no parser, semantic analyzer e codegen. Nenhuma alteração de código deve ser considerada final sem a validação de testes automatizados.

## 🚀 Gatilho (Trigger)
Esta skill deve ser ativada SEMPRE que você:
1. Iniciar uma nova feature (ex: nova estrutura de controle, novo tipo).
2. Tentar corrigir um bug reportado.
3. Refatorar qualquer parte crítica do compilador (Lexer, Parser, Binder).

## 📖 Instruções de Execução

### 1. Reprodução / Teste de Base
Antes de alterar o código:
- Se for um bug: Crie um arquivo em `tests/` que reproduza a falha.
- Se for uma feature: Crie um arquivo de teste descrevendo o comportamento esperado.

### 2. Execução da Suite
Rode todos os testes existentes para garantir que o estado atual é estável:
```bash
lua tools/test_all.lua
```

### 3. Implementação e Validação
- Implemente as mudanças no compilador.
- Rode o comando de teste específico para o componente alterado (ex: `lua tests/semantic_tests/test_semantic.lua`).
- Quando passar, rode a suite completa novamente (`test_all.lua`).

### 4. Snapshots (Opcional)
Se a mudança afetar a saída do compilador (Lua gerado), verifique o diretório `tests/snapshot_tests` para garantir que o output é idêntico ao esperado.

---

## 💡 Dica de Ouro
Se você estiver em dúvida se uma mudança quebrou algo, **rode os testes**. O Zenith foi construído para que os testes sejam rápidos e informativos.
