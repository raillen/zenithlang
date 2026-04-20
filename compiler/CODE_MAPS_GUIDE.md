# 📘 Guia de Uso - Code Maps

## 🎯 O Que São Code Maps?

Code Maps são arquivos de mapeamento documental que catalogam cada módulo do código com:
- **Funções críticas** e suas responsabilidades
- **Dependencies** entre módulos
- **Pontos de falha** e o que pode quebrar
- **Estado crítico** que precisa ser monitorado

## Fluxo Recomendado

1. Rode `python tools/generate_code_maps.py`
2. Abra o mapa do módulo relevante
3. Use o bloco `Generated Index` para achar `arquivo`, `símbolo` e `linha`
4. Preencha manualmente só o que a automação não sabe:
   - risco real
   - regressões comuns
   - testes que faltam
   - decisões importantes

## 📂 Estrutura de Arquivos

```
compiler/
├── CODE_MAP.md                    ← Índice mestre
├── driver/
│   └── DRIVER_MAP.md             ← Mapa do driver/CLI
├── frontend/
│   ├── lexer/LEXER_MAP.md        ← Mapa do lexer
│   ├── parser/PARSER_MAP.md      ← Mapa do parser
│   └── ast/AST_MAP.md            ← Mapa do AST
├── zir/
│   ├── ZIR_MODEL_MAP.md          ← Mapa do model ZIR
│   ├── ZIR_PARSER_MAP.md         ← Mapa do parser ZIR
│   └── ZIR_VERIFIER_MAP.md       ← Mapa do verifier
└── ... (cada módulo tem seu MAP.md)
```

## 🚀 Como Preencher os Mapas

### Passo 1: Escolha um Módulo

Comece pelos módulos **CRÍTICOS** (🔴):
1. Driver/CLI (`main.c` - 147KB!)
2. Lexer
3. Parser
4. ZIR Model
5. Semantic Binder
6. Type System
7. C Emitter
8. Runtime

### Passo 2: Gere o Índice Automático

Comando:

```bash
python tools/generate_code_maps.py
```

Opcional, para um módulo só:

```bash
python tools/generate_code_maps.py --match binder
```

### Passo 3: Use IA para Completar as Notas Manuais

**Prompt sugerido:**
```
Analise o arquivo [caminho/arquivo.c] usando o bloco Generated Index do
[MAP.md correspondente] e preencha só as notas manuais relevantes.

Para cada função, forneça:
- O que faz em 1 frase
- O que pode quebrar
- Testes relacionados
- Dependências importantes
- Prioridade prática do problema

Não repita o que já está no bloco gerado.
```

### Passo 4: Valide Manualmente

- [ ] Confira se as linhas estão corretas
- [ ] Adicione contexto que a IA não sabe
- [ ] Marque erros comuns que você já viu
- [ ] Adicione notas de manutenção específicas

### Passo 5: Use no Dia a Dia

**Para Debug:**
```
1. Erro em binder.c:347?
2. Abre semantic/binder/BINDER_MAP.md
3. Encontra função na tabela
4. Vê dependencies e "pode quebrar se"
5. Reporta bug para IA com contexto rico
```

**Para Features Novas:**
```
1. Quer adicionar feature X?
2. Consulta mapas para entender onde mexer
3. Vê dependencies antes de modificar
4. Evita efeitos colaterais
```

## 💡 Exemplo de Uso com IA

### ❌ SEM Code Map (gasta muitos tokens):
```
"Tenho um erro no binder.c linha 347, aqui está o arquivo inteiro
(2000 linhas), pode me ajudar a debugar?"

→ Tokens: ~8000 (file) + ~500 (prompt) = 8500 tokens
```

### ✅ COM Code Map (economia de 80%):
```
"Erro em binder.c:347. Segundo o BINDER_MAP.md, essa linha está na 
função bind_variable_declaration() que:
- Responsabilidade: bind var declarations ao scope atual
- Dependencies: symbol_table, type_checker
- Pode quebrar se: variable já declarada no mesmo scope

O erro é 'duplicate declaration'. Aqui estão as linhas 340-355."

→ Tokens: ~200 (contexto) + ~100 (código) + ~50 (prompt) = 350 tokens
```

**Economia: 96% menos tokens!** 🎉

## 📊 Métricas de Progresso

Atualize o `CODE_MAP.md` conforme o trabalho real:

| Status | Significado |
|--------|-------------|
| ⬜ Pendente | Mapa ainda não existe |
| ⚙️ Gerado | Bloco automático atualizado |
| 📝 Manual parcial | Notas manuais começaram |
| ✅ Validado | Notas manuais revisadas |

## 🎯 Próximos Passos Recomendados

1. **Hoje**: Rode o gerador e revise 2-3 mapas críticos
2. **Semana 1**: Complete notas manuais dos 3 módulos mais críticos
3. **Semana 2**: Valide essas notas com testes e bugs reais
4. **Semana 3-4**: Expanda para os outros módulos
5. **Contínuo**: Rode o gerador novamente após mudanças estruturais

## ⚠️ Dicas Importantes

### O Que Incluir nos Mapas:
- ✅ Funções que você já debugou antes
- ✅ Código que frequentemente quebra
- ✅ Dependencies não óbvias
- ✅ Workarounds e hacks conhecidos
- ✅ Ordem de execução importante

### O Que NÃO Incluir:
- ❌ Código óbvio (getters/setters simples)
- ❌ Comentários que mudam a cada refatoração
- ❌ Detalhes que o código já mostra claramente
- ❌ Tudo (seja seletivo, senão vira overhead)

## 🔥 Modo Emergencial (Tempo Limitado)

Se tem pouco tempo, mapeie **SÓ** isso:

1. **Funções que já te causaram bugs**
2. **Dependencies críticas** (o que quebra se mudar X)
3. **Erros comuns** (FAQ de debugging)

Isso já te dá 80% do benefício com 20% do esforço.

---

**Perguntas?** Abra uma issue ou pergunte para a IA! 🤖
