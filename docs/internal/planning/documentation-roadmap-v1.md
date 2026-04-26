# Zenith Documentation Roadmap v1

> Audience: maintainer
> Status: proposed
> Surface: internal
> Source of truth: yes

## Objetivo

Transformar a documentacao Zenith em um sistema completo, explicito e facil de
consumir.

O usuario deve conseguir:

- aprender a linguagem em ordem;
- consultar qualquer recurso sem procurar em decisoes internas;
- entender como Zenith resolve problemas comuns;
- encontrar exemplos pequenos e exemplos completos;
- saber o que existe hoje, o que e futuro e o que nao sera implementado.

O objetivo nao e escrever mais texto por volume.

O objetivo e reduzir atrito cognitivo.

## Escopo

Entra neste plano:

- documentacao publica para usuarios;
- referencia completa da linguagem;
- cookbook de problemas comuns;
- exemplos executaveis;
- padrao editorial para TDAH e dislexia;
- rastreio de cobertura documental por feature.

Nao entra neste plano:

- mudar semantica da linguagem;
- reabrir decisoes aceitas;
- transformar roadmap interno em tutorial;
- publicar features futuras como se ja fossem atuais.

## Dependencias

- Upstream:
  - `docs/DOCS-STRUCTURE.md`
  - `docs/internal/standards/documentation-style-guide.md`
  - `language/spec/README.md`
  - `language/decisions/README.md`
  - `tests/behavior/MATRIX.md`
- Downstream:
  - `docs/public/`
  - `docs/reference/`
  - `docs/wiki/`
  - `docs/internal/planning/documentation-checklist-v1.md`
- Codigo/Testes relacionados:
  - `tools/check_docs_paths.py`
  - `tests/behavior/MATRIX.md`
  - `language/spec/implementation-status.md`

## Conteudo principal

### D1 - Separar publico, referencia e spec

Cada camada deve ter uma funcao clara.

| Camada | Funcao | Exemplo |
| --- | --- | --- |
| `docs/public/learn/` | ensinar em ordem | "Primeiro programa", "Tipos", "Erros" |
| `docs/public/cookbook/` | resolver problemas comuns | "Como representar ausencia" |
| `docs/reference/language/` | consulta completa | regras, exemplos e limites |
| `docs/reference/stdlib/` | consulta da stdlib | modulo, funcoes, erros |
| `language/spec/` | contrato normativo | parser, checker, runtime |
| `language/decisions/` | historico e motivo | tradeoffs e contexto |

Regra:

- guia ensina;
- referencia consulta;
- spec decide;
- decision explica o motivo.

### D2 - Criar manual completo da linguagem

O manual deve cobrir todos os recursos que o usuario encontra na linguagem.

Topicos minimos:

- forma de arquivo;
- namespace e imports;
- comentarios;
- tipos primitivos;
- `const`, `var` e mutabilidade;
- funcoes;
- parametros nomeados e defaults;
- controle de fluxo;
- `optional`;
- `result`;
- `?`;
- `panic`;
- `check`;
- texto, bytes e `fmt`;
- listas e mapas;
- structs;
- field defaults;
- field/parameter `where`;
- traits;
- `apply`;
- enums com payload;
- generics;
- constraints;
- callables/delegates;
- closures e lambdas suportadas;
- `lazy<T>`;
- `dyn<Trait>`;
- FFI atual;
- ZPM/projeto;
- testes;
- formatter;
- diagnosticos.

Cada topico deve ter:

1. quando usar;
2. forma canonica;
3. exemplo minimo;
4. exemplo real pequeno;
5. erro comum;
6. como corrigir;
7. diferenca para outras linguagens quando isso evitar confusao;
8. limite atual.

### D3 - Criar cookbook de problemas comuns

O cookbook responde perguntas do tipo "como faco X em Zenith?".

Topicos recomendados:

- como representar ausencia sem `null`;
- como representar erro sem exception;
- como fazer union segura;
- como fazer metodo abstrato;
- como fazer dispatch virtual;
- como separar metodos de um tipo sem partial class;
- como validar entrada;
- como converter para texto;
- como concatenar e interpolar texto;
- como fazer lookup seguro em list/map;
- como lidar com valor opcional;
- como lidar com resultado falho;
- como escrever uma API publica pequena;
- como escolher entre `match`, `if`, `?` e helper;
- como escrever testes legiveis.

Cada receita deve ter:

- problema;
- resposta curta;
- codigo;
- explicacao;
- erro comum;
- quando nao usar.

### D4 - Criar trilhas de leitura

Nem todo usuario deve ler tudo em ordem.

Trilhas recomendadas:

- "Quero rodar meu primeiro programa".
- "Quero aprender a linguagem".
- "Quero escrever uma lib".
- "Quero entender erros e diagnosticos".
- "Quero migrar mentalmente de C#/Java/TypeScript/Rust/Go".
- "Quero contribuir no compilador".

Cada trilha deve ter no maximo 7 passos visiveis por pagina.

### D5 - Reescrever referencia para ser completa, mas escaneavel

A referencia deve ser exaustiva sem virar parede de texto.

Padrao por pagina:

- resumo de 3 linhas;
- tabela de formas validas;
- exemplos pequenos;
- regras;
- erros comuns;
- limites atuais;
- links para spec/decision;
- links para behavior tests quando houver.

### D6 - Criar matriz de cobertura documental

Cada feature deve ter cobertura marcada.

Campos minimos:

| Feature | Guia | Referencia | Cookbook | Spec | Teste | Status |
| --- | --- | --- | --- | --- | --- | --- |

Status permitido:

- `complete`;
- `partial`;
- `missing`;
- `future`;
- `historical`.

### D7 - Validar acessibilidade textual

Regras obrigatorias:

- paragrafos curtos;
- subtitulos frequentes;
- uma ideia principal por bloco;
- exemplos antes de teoria longa quando possivel;
- tabelas para comparacao;
- listas curtas;
- evitar jargao sem definicao;
- separar "existe hoje", "futuro" e "nao entra".

### D8 - Sincronizar com o compilador

Documentacao publica nao pode prometer recurso que o compilador nao aceita.

Todo topico de linguagem deve indicar:

- `current`: existe e tem teste;
- `partial`: existe com limites;
- `planned`: decisao aceita, nao pronto;
- `rejected`: nao faz parte da filosofia atual.

## Validacao

Antes de fechar uma etapa:

```powershell
python tools/check_docs_paths.py
git diff --check
```

Para topicos de linguagem:

- conferir `tests/behavior/MATRIX.md`;
- conferir `language/spec/implementation-status.md`;
- incluir exemplo que compile ou marcar explicitamente como ilustrativo.

## Historico de atualizacao

- 2026-04-25: plano inicial criado apos identificacao de lacunas de ergonomia,
  completude e acessibilidade na documentacao de usuario.
