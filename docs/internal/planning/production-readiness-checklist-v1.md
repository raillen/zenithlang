# Zenith Production Readiness Checklist 1.0

> Checklist operacional derivado de
> `docs/internal/planning/production-readiness-roadmap-v1.md`.
> Status: ativo para maturidade de producao.
> Criado: 2026-04-26.

## Como usar

1. Marque apenas itens com evidencia no codigo, docs, testes ou release.
2. Se um item falhar em clean clone, ele continua aberto.
3. Nao feche item de linguagem sem spec, fixture e doc quando ele for publico.
4. Se aparecer feature nova grande, mova para roadmap futuro em vez de colocar
   dentro do caminho de producao.
5. Mantenha o texto curto e direto para reduzir carga cognitiva.

## Estado alvo

- [ ] Zenith esta com superficie 1.0 congelada.
- [ ] A stdlib publica esta marcada como `stable`, `experimental` ou `internal`.
- [ ] O repositorio nao depende de artefatos locais nao documentados.
- [ ] Uma maquina limpa consegue compilar, testar e rodar exemplos oficiais.
- [ ] Nao ha P0/P1 aberto em compilador, runtime, stdlib estavel, formatter, LSP
      ou CLI.
- [ ] Release candidate 0.9 foi publicado e validado antes de 1.0.

## Gates obrigatorios

- [ ] `python build.py` verde.
- [ ] `./zt.exe check zenith.ztproj --all --ci` verde.
- [ ] `python run_suite.py smoke --no-perf` verde.
- [ ] `python run_suite.py pr_gate --no-perf` verde.
- [ ] Gates rodam em Windows.
- [ ] Gates rodam em Linux antes de 1.0.
- [ ] Gate de release roda a partir de clone limpo.
- [ ] Falha de gate mostra causa e proxima acao.

## P0 - Higiene de repositorio e release

- [ ] Auditar arquivos grandes versionados.
- [ ] Decidir remover historico pesado ou usar Git LFS para artefatos reais.
- [ ] Garantir `.gitignore` para `target/`.
- [ ] Garantir `.gitignore` para `.ztc-tmp/`.
- [ ] Garantir `.gitignore` para binarios gerados.
- [ ] Garantir `.gitignore` para `.pdb`.
- [ ] Garantir `.gitignore` para `.rlib`.
- [ ] Documentar que build output nao entra em commit.
- [ ] Validar build em clone limpo.
- [ ] Validar release package em diretorio separado do checkout principal.
- [ ] Manter PR/release com worktree limpo ou escopo explicitamente auditado.

Criterio de aceite:

- [ ] Release e recriavel por comando documentado.
- [ ] O repositorio nao precisa de arquivos locais escondidos.
- [ ] Artefatos gerados nao entram por acidente em commit.

## P1 - Superficie 1.0 da linguagem

- [ ] Listar keywords aceitas para 1.0.
- [ ] Listar tipos primitivos aceitos para 1.0.
- [ ] Listar builtins aceitos para 1.0.
- [ ] Documentar `to_text(value)` como builtin do compilador.
- [ ] Separar conversoes futuras como API estavel, trait core ou experimento.
- [ ] Definir comportamento publico de `result<T,E>`.
- [ ] Definir comportamento publico de `optional<T>`.
- [ ] Definir regras de namespace e arquivo.
- [ ] Definir regras de `for`, incluindo que nao existe `for` top-level.
- [ ] Definir o que fica fora de 1.0.
- [ ] Bloquear nova sintaxe grande durante estabilizacao, salvo decisao formal.

Criterio de aceite:

- [ ] Leitor consegue distinguir linguagem estavel, stdlib e experimento.
- [ ] Toda mudanca publica altera spec, fixture e doc no mesmo ciclo.

## P2 - Diagnosticos e erros

- [ ] Erros comuns mostram mensagem action-first.
- [ ] Erro de `for` top-level sugere mover codigo para `func`.
- [ ] Erro de `string` sugere `text`.
- [ ] Erro de `let` sugere `const` ou `var`.
- [ ] Erro de `&&` sugere `and`.
- [ ] Erro de `||` sugere `or`.
- [ ] Erro de `!value` sugere `not value`.
- [ ] Erro de `null` sugere `optional<T>` e `none`.
- [ ] Erro de `throw` sugere `result<T,E>`, `error(...)` ou `panic(...)`.
- [ ] Erros com namespaces mostram caminho esperado quando possivel.
- [ ] Diagnosticos usados pelo LSP seguem o mesmo pipeline do CLI.
- [ ] Fixtures invalidas cobrem diagnosticos publicos.

Criterio de aceite:

- [ ] Usuario consegue corrigir o erro sem abrir codigo do compilador.

## P3 - Compilador, runtime e backend

- [ ] Binder nao aceita simbolo publico sem teste.
- [ ] Type checker valida builtins com aridade e tipo corretos.
- [ ] Lowering cobre cada builtin publico.
- [ ] Backend C gera codigo previsivel para caminhos estaveis.
- [ ] Runtime tem limites documentados.
- [ ] Panics e falhas fatais mostram mensagem curta.
- [ ] Erros recuperaveis usam `result<T,E>` onde a API publica prometer isso.
- [ ] Testes unitarios cobrem regressao de tipo.
- [ ] Behavior tests cobrem fluxo executavel.
- [ ] Conformance tests cobrem regras de linguagem.

Criterio de aceite:

- [ ] Feature publica so fecha quando passa por parser/binder/typecheck/lowering
      e execucao quando aplicavel.

## P4 - Stdlib essencial

- [ ] `core` estavel para erro base, `result`, `optional` e contratos centrais.
- [ ] `std.io` com docs, exemplos e erros claros.
- [ ] `std.text` com helpers comuns e comportamento documentado.
- [ ] `std.collections` com lista, mapa e consultas basicas.
- [ ] `std.fs` com leitura/escrita simples e erros recuperaveis.
- [ ] `std.time` com tempo basico sem ambiguidade.
- [ ] `std.os` ou `std.process` com limites documentados.
- [ ] `std.test` com assertions legiveis.
- [ ] Cada modulo estavel tem behavior tests.
- [ ] Cada modulo estavel tem exemplo curto.
- [ ] Cada modulo estavel tem doc publica.

Criterio de aceite:

- [ ] Projeto pequeno consegue usar texto, arquivos, colecoes, erros e testes sem
      depender de modulo interno.

## P5 - Tooling

- [ ] Extensao VS Code empacotavel.
- [ ] Extensao cria namespace automatico para arquivo novo vazio em `source.root`.
- [ ] Configuracao permite desligar namespace automatico.
- [ ] LSP atualiza diagnosticos de arquivo novo sem restart manual.
- [ ] Hover funciona para simbolos estaveis.
- [ ] Completion nao sugere APIs internas por padrao.
- [ ] Formatter e idempotente.
- [ ] Golden tests cobrem regras principais do formatter.
- [ ] `zt explain <codigo>` existe para erros comuns.
- [ ] CLI mantem texto curto por padrao.
- [ ] CLI tem escape hatch para output bruto quando necessario.
- [ ] Fluxo de pacote/projeto cobre init, add, run e check.

Criterio de aceite:

- [ ] Usuario consegue criar projeto, criar arquivo, receber diagnostico e rodar
      check sem conhecer layout interno do compilador.

## P6 - Documentacao e exemplos

- [ ] Pagina de instalacao atualizada.
- [ ] Guia "primeiro projeto" atualizado.
- [ ] Guia de namespaces atualizado.
- [ ] Guia de tipos e conversoes atualizado.
- [ ] Guia de builtins atualizado, incluindo `to_text`.
- [ ] Guia de erros e `result<T,E>` atualizado.
- [ ] Guia de stdlib essencial atualizado.
- [ ] Exemplos oficiais rodam em clean clone.
- [ ] Exemplos usam apenas APIs publicas ou avisam quando usam experimento.
- [ ] Roadmap e checklist ativos apontam para este plano de producao.

Criterio de aceite:

- [ ] Usuario novo consegue seguir docs sem depender de conversa, memoria local ou
      conhecimento interno do repo.

## P7 - Compatibilidade e release

- [ ] SemVer documentado.
- [ ] Politica pre-1.0 documentada.
- [ ] Politica pos-1.0 documentada.
- [ ] Changelog obrigatorio por release.
- [ ] Template de release notes existe.
- [ ] Breaking changes exigem secao clara de migracao.
- [ ] APIs depreciadas tem prazo minimo antes de remocao.
- [ ] Limites conhecidos publicados.
- [ ] Release candidate 0.9 publicado antes de 1.0.
- [ ] 1.0 so sai depois de periodo sem P0/P1 novo.

Criterio de aceite:

- [ ] Atualizar Zenith tem risco previsivel e instrucao de migracao.

## P8 - Dogfooding com Borealis

- [ ] Borealis compila usando o SDK/release esperado.
- [ ] Borealis nao depende de caminhos locais do checkout principal.
- [ ] Cenarios de Borealis cobrem projeto Zenith nao trivial.
- [ ] Falhas de Borealis viram issue, fixture ou decisao.
- [ ] Regressao em area estavel bloqueia RC quando quebrar Borealis.
- [ ] Exemplos Borealis rodam de clean clone quando marcados como oficiais.

Criterio de aceite:

- [ ] Zenith e usada em projeto grande o bastante para revelar problemas reais de
      linguagem, stdlib e tooling.

## P9 - Decisao final de producao publica

- [ ] Checklist inteiro revisado.
- [ ] P0/P1 zerados.
- [ ] Gate completo verde em clean clone.
- [ ] Docs publicas revisadas.
- [ ] Release package instalado em maquina sem estado local.
- [ ] Exemplos oficiais executados a partir do pacote publicado.
- [ ] Limites conhecidos publicados.
- [ ] Tag 1.0 criada.
- [ ] Artefatos 1.0 publicados.
- [ ] Changelog 1.0 publicado.

Criterio de aceite:

- [ ] A decisao de producao e baseada em evidencia reproduzivel, nao em sensacao
      de maturidade.

