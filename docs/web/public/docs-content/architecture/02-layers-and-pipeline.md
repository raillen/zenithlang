# Zenith Implementation: Camadas e Pipeline

## Resumo rápido
Este módulo mostra como o compilador Zenith se organiza em camadas e como os dados percorrem o pipeline de compilação, do texto-fonte até o código Lua final.

## Quando usar
Leia este documento quando precisar entender a ordem das fases, descobrir em qual camada uma regra deve viver ou revisar a estrutura de pastas do compilador.

## Pré-requisitos
- Ter lido a visão geral e os princípios do compilador.
- Saber diferenciar análise sintática, análise semântica e geração de código.

## Tempo de leitura
10 a 12 minutos.

## Exemplo mínimo
```text
arquivo .zt
  -> source/
  -> diagnostics/
  -> syntax/
  -> semantic/
  -> lowering/
  -> backend/lua/
```

## Estrutura de alto nível

O compilador Zenith é um pipeline de múltiplas passagens. Cada etapa recebe uma representação mais rica do que a anterior e entrega uma forma mais específica para a etapa seguinte.

Fluxo recomendado:

1. Ler o arquivo-fonte.
2. Tokenizar.
3. Construir a AST sintática.
4. Resolver nomes, tipos e escopos.
5. Reduzir açúcar sintático.
6. Emitir Lua.

## Estrutura de pastas

```text
zenith/
├── docs/
├── examples/
├── testdata/
├── src/
│   ├── cli/
│   ├── common/
│   ├── source/
│   ├── diagnostics/
│   ├── syntax/
│   ├── semantic/
│   ├── lowering/
│   ├── backend/
│   │   └── lua/
│   ├── project_system/
│   └── pipeline/
├── tests/
└── tools/
```

Essa estrutura separa claramente:

- o texto de entrada;
- o modelo interno do compilador;
- o backend;
- o sistema de projeto;
- os testes por fase.

## As camadas e suas responsabilidades

### `source/`

Responsável por representar o arquivo bruto e a localização de trechos de texto.

Peças centrais:

- `source_file`;
- `source_text`;
- `span`;
- `location`;
- `line_map`.

Invariante: nenhum outro módulo acessa texto-fonte “na mão”. Tudo passa por `source_text` e `span`.

### `diagnostics/`

Centraliza erros, avisos e dicas produzidos pelas fases do compilador.

Peças centrais:

- `diagnostic`;
- `diagnostic_bag`;
- `diagnostic_code`;
- `diagnostic_reporter`;
- `diagnostic_formatting`.

Saída desejada:

```text
Erro [ZT-001]: variável 'x' não declarada
  --> src/main.zt:12:5
   |
12 |     x = 10
   |     ^ símbolo não encontrado
Dica: você quis dizer 'count'?
```

Invariante: todo diagnóstico deve apontar para um `span` preciso. Mensagens vagas como “erro de sintaxe” não bastam.

### `syntax/`

Cuida da forma do código. Esta camada sabe como o texto está escrito, mas não sabe ainda o que ele significa.

Subáreas principais:

- `tokens/`;
- `lexer/`;
- `ast/`;
- `parser/`.

### `semantic/`

Resolve o significado do programa. Aqui o compilador descobre:

- que símbolo um identificador representa;
- quais tipos estão envolvidos;
- quais regras de escopo e mutabilidade se aplicam;
- quais erros semânticos precisam ser emitidos.

### `lowering/`

Transforma construções ricas da linguagem em uma forma mais simples antes da geração de código.

Exemplos típicos:

- `repeat N times`;
- interpolação de string;
- `attempt/rescue`;
- `watch`;
- `expr!`;
- desestruturação.

### `backend/lua/`

É a única camada que conhece Lua. Recebe uma IR já estabilizada e produz texto Lua válido.

Responsabilidades centrais:

- respeitar Lua 5.4;
- escapar nomes reservados;
- emitir helpers de runtime;
- manter a equivalência semântica com o programa Zenith.

## Pipeline de compilação

O pipeline deve ser lido como uma sequência de decisões cada vez mais específicas:

1. `source/` captura o arquivo e localizações.
2. `diagnostics/` oferece um formato comum para erros.
3. `syntax/lexer` transforma caracteres em tokens.
4. `syntax/parser` transforma tokens em AST.
5. `semantic/` valida nomes, tipos e escopos.
6. `lowering/` reduz açúcar sintático.
7. `backend/lua/` constrói a saída final.

## Regras de arquitetura que não devem ser quebradas

- O parser não resolve nomes.
- O binder não emite Lua.
- O emitter não corrige erro semântico.
- O lowering não deve reinventar análise de tipos.
- O sistema de projeto não deve conter lógica sintática ou semântica.

## Onde cada tipo de mudança deve entrar

Use este guia rápido:

- **Novo token, palavra-chave ou forma textual**: `syntax/tokens` e `syntax/lexer`.
- **Nova estrutura gramatical**: `syntax/parser` e `syntax/ast`.
- **Nova regra de escopo ou tipos**: `semantic/`.
- **Novo açúcar sintático**: `lowering/`.
- **Novo padrão de emissão Lua**: `backend/lua/`.
- **Nova convenção de projeto ou build**: `project_system/` ou `pipeline/`.

## Resumo operacional

Se você estiver em dúvida sobre onde implementar algo, pergunte primeiro:

> isso muda forma, significado, simplificação ou emissão?

Essa pergunta normalmente basta para localizar a mudança na camada certa e evitar acoplamentos difíceis de manter.
