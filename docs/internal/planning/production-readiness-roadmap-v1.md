# Zenith Production Readiness Roadmap 1.0

> Roadmap final para levar Zenith de alpha avancado / beta inicial ate producao
> publica.
> Status: ativo para maturidade de producao.
> Criado: 2026-04-26.

## Decisao atual

Zenith ainda nao deve ser anunciada como linguagem pronta para producao publica.

O estado correto hoje e:

- alpha avancado para pesquisa, dogfooding e desenvolvimento interno;
- beta inicial para projetos controlados, com dono tecnico perto do compilador;
- ainda nao e 1.0 para sistemas externos com promessa de compatibilidade longa.

Uso recomendado agora:

- exemplos oficiais;
- pacotes internos;
- Borealis e outros projetos de dogfooding;
- validacao de sintaxe, ergonomia, stdlib e tooling.

Uso ainda nao recomendado:

- produto critico sem plano de contingencia;
- API publica prometida como estavel;
- dependencia externa sem controle do time Zenith;
- release feita a partir de arvore suja ou com artefatos gerados versionados.

## Definicao de producao publica

Zenith so entra em producao publica quando todos estes pontos forem verdadeiros:

- uma copia limpa do repositorio compila e passa os gates principais;
- nao existem arquivos gerados pesados versionados por acidente;
- a superficie 1.0 da linguagem esta congelada;
- a stdlib marcada como estavel tem testes, docs e diagnosticos minimos;
- a extensao/editor funciona sem depender de caminhos locais do repo;
- releases sao reproduziveis;
- mudancas incompativeis seguem politica clara;
- nao ha bugs P0/P1 abertos no compilador, runtime, formatter, LSP ou stdlib
  estavel.

## Linha de milestones

| Marco | Objetivo | Saida esperada |
| --- | --- | --- |
| 0.4 beta de estabilizacao | Parar crescimento solto e estabilizar o que ja existe. | Gates confiaveis, repo limpo, backlog P0/P1 visivel. |
| 0.5 beta de stdlib | Deixar a stdlib essencial previsivel. | Modulos core com docs, exemplos e behavior tests. |
| 0.6 beta de tooling | Tornar o ciclo diario confortavel. | VS Code extension empacotavel, LSP estavel, formatter idempotente. |
| 0.7 beta de compatibilidade | Definir contrato de mudanca. | SemVer, changelog, politica de breaking changes e labels stable/experimental/internal. |
| 0.8 beta de dogfooding | Provar Zenith em projeto real maior. | Borealis e exemplos oficiais rodando de clean clone. |
| 0.9 release candidate | Congelar 1.0 e aceitar apenas correcao. | RC publicado com checklist final fechado. |
| 1.0 producao publica | Prometer uso publico com contrato. | Release tagueado, docs publicas, gate verde e limites conhecidos publicados. |

## P0 - Higiene de repositorio e release

Objetivo: release precisa sair de um estado limpo, pequeno e reproduzivel.

Entregas:

- auditar arquivos grandes ja versionados;
- decidir entre remover historico pesado ou migrar artefatos reais para LFS;
- garantir `.gitignore` para `target/`, `.ztc-tmp/`, binarios, `.pdb`, `.rlib`
  e saidas de build;
- documentar politica: build output nao entra em commit;
- validar release a partir de clone limpo;
- separar artefato de release do diretorio de desenvolvimento local.

Criterio de aceite:

- clone limpo consegue compilar sem arquivos locais escondidos;
- nenhum gate depende de arquivo gerado versionado por acidente;
- pacote de release e recriavel por comando documentado.

## P1 - Congelamento da superficie 1.0

Objetivo: deixar claro o que e linguagem, o que e stdlib e o que e experimental.

Entregas:

- listar keywords, tipos primitivos, builtins e regras semanticas aceitas para
  1.0;
- marcar modulos como `stable`, `experimental` ou `internal`;
- documentar builtins que precisam do compilador, como `to_text`;
- evitar nova sintaxe grande ate fechar o ciclo de estabilizacao;
- exigir spec, teste e doc para cada comportamento publico.

Criterio de aceite:

- usuario consegue saber se uma feature e promessa de compatibilidade ou
  experimento;
- mudancas de linguagem nao entram sem atualizar spec, fixtures e docs.

## P2 - Gates obrigatorios e CI

Objetivo: reduzir regressao e tornar a qualidade verificavel.

Gates minimos locais:

```powershell
python build.py
./zt.exe check zenith.ztproj --all --ci
python run_suite.py smoke --no-perf
python run_suite.py pr_gate --no-perf
```

Entregas:

- rodar gates em Windows;
- adicionar runner Linux antes de 1.0;
- separar smoke rapido de gate completo;
- bloquear merge quando P0/P1 falhar;
- registrar fixtures invalidas para diagnosticos publicos;
- manter `tests/behavior/MATRIX.md` como mapa de cobertura funcional.

Criterio de aceite:

- PR comum tem feedback rapido;
- release candidate so sai com gate completo verde;
- falha de fixture aponta para causa e proxima acao.

## P3 - Stdlib essencial

Objetivo: tornar os modulos basicos previsiveis para programas reais.

Escopo minimo estavel:

- `core`: `result`, `optional`, erro base e contratos centrais;
- `std.io`: impressao, entrada quando suportada e erros claros;
- `std.text`: operacoes comuns de texto;
- `std.collections`: lista, mapa e helpers de consulta;
- `std.fs`: leitura/escrita simples com erro recuperavel;
- `std.time`: tempo basico sem ambiguidade;
- `std.os` / `std.process`: ambiente e processo com limites documentados;
- `std.test`: helpers legiveis para testes.

Entregas:

- docs curtas por modulo;
- exemplos executaveis;
- behavior tests positivos;
- fixtures negativas para erros comuns;
- politica de estabilidade por modulo.

Criterio de aceite:

- um projeto pequeno consegue usar arquivos, texto, colecoes, erros e testes sem
  depender de detalhes internos.

## P4 - Tooling e experiencia diaria

Objetivo: programar em Zenith deve ser previsivel fora do checkout do compilador.

Entregas:

- empacotar a extensao VS Code;
- garantir namespace automatico em arquivos novos quando houver `source.root`;
- estabilizar LSP para erro, hover, completion e diagnostico incremental;
- formatter idempotente: `fmt(fmt(x)) == fmt(x)`;
- comando `zt explain <codigo>` para erros comuns;
- fluxo minimo de pacote/projeto: init, add, run e check;
- mensagens de CLI curtas, legiveis e com escape hatch para output bruto.

Criterio de aceite:

- usuario instala ferramenta, cria projeto, escreve arquivo novo e roda check sem
  conhecer a estrutura interna do repo.

## P5 - Compatibilidade e contrato de release

Objetivo: usuario precisa saber quando pode atualizar sem medo.

Entregas:

- SemVer documentado;
- changelog obrigatorio por release;
- politica clara para breaking changes antes e depois de 1.0;
- periodo minimo de depreciacao para APIs estaveis;
- labels publicas: `stable`, `experimental`, `internal`;
- pagina de limites conhecidos.

Criterio de aceite:

- cada release explica o que mudou, o que quebrou e qual acao o usuario deve
  tomar.

## P6 - Dogfooding com Borealis e exemplos oficiais

Objetivo: provar que Zenith aguenta um projeto maior que exemplos pequenos.

Entregas:

- Borealis deve compilar usando o SDK/release esperado;
- exemplos oficiais devem rodar de clean clone;
- regressao em Borealis deve bloquear RC quando tocar area estavel;
- cenarios reais devem alimentar diagnosticos, stdlib e tooling;
- documentar lacunas encontradas durante dogfooding.

Criterio de aceite:

- um projeto Zenith nao trivial roda sem atalhos locais;
- lacunas descobertas viram issue, fixture ou decisao documentada.

## P7 - Release candidate e decisao 1.0

Objetivo: transformar maturidade tecnica em release publico confiavel.

Entregas para 0.9 RC:

- checklist de readiness fechado;
- backlog P0/P1 zerado;
- release notes do RC;
- docs publicas revisadas;
- exemplos revisados em clean clone;
- pacote de release testado em maquina sem estado local.

Entregas para 1.0:

- tag final;
- artefatos publicados;
- changelog final;
- pagina de instalacao;
- politica de suporte;
- limites conhecidos publicados.

Criterio de aceite:

- a decisao de 1.0 nao depende de confianca subjetiva; depende de checklist
  fechado e evidencia reproduzivel.
