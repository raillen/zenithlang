# Zenith Performance Audit Skill

Esta skill descreve as diretrizes para otimizar o código Lua gerado pelo compilador Zenith para máxima eficiência.

## 🎯 Objetivo
Garantir que o código Lua transpilado pelo Zenith seja tão performático quanto o código Lua escrito à mão, evitando gargalos comuns da VM da Lua.

## 🚀 Gatilho (Trigger)
Esta skill deve ser ativada SEMPRE que você:
1. Alterar a lógica de emissão (codegen) no compilador.
2. Implementar uma nova estrutura de dados nativa (ex: `grid`, `uniq`).
3. Notar lentidão em benchmarks de larga escala.

## 📖 Instruções de Execução

### 1. Checklist de Otimização Lua
Siga este checklist ao emitir código Lua:
- **Tabelas:** Evite criar tabelas desnecessárias. Se possível, reuse buffers.
- **Concatenação:** Se estiver juntando muitas strings, use `table.concat` em vez do operador `..`.
- **Globals:** Aplique `local print = print` no topo de arquivos Lua quando a função for muito usada (cache de globais).
- **Tipagem Dinâmica:** Aproveite as certezas do sistema de tipos do Zenith para evitar `type()` ou guards dinâmicos excessivos no código gerado.

### 2. Auditoria de Loops
Loops em Zenith devem ser transpilados para loops numéricos ou genéricos simples da Lua sempre que possível. Evite closures dentro de loops apertados se elas puderem ser pré-definidas.

### 3. Gerenciamento de Memória
Garanta que o código gerado facilite o trabalho do coletor de lixo da Lua. Limpe referências em estruturas globais ou persistentes.

### 4. Benchmarking
Sempre que fizer uma otimização:
1. Rode um teste de performance comparando o tempo de execução antes e depois.
2. Documente o ganho no `CHANGELOG.md` na seção `Performance`.

---

## 💡 Dica de Ouro
A Lua é rápida por design. O Zenith deve apenas "sair do caminho" e deixar a VM fazer seu trabalho de forma eficiente.
