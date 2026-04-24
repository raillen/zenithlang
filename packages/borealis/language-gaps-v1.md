# Borealis Language Gaps v1

Data: 2026-04-23  
Escopo: mudancas futuras desejadas na Zenith para permitir que o Borealis chegue na forma ideal sem empurrar complexidade para o package.

## Como ler este arquivo

Este documento separa duas coisas:

- o que ja conseguimos entregar agora no `packages/borealis`;
- o que ainda seria melhor resolver na linguagem, no compilador ou no toolchain.

Regra de governanca:

- Borealis continua evoluindo como package;
- gaps estruturais ficam registrados aqui para Zenith Core;
- nao devemos colocar hacks especiais para o Borealis no compilador.

## Gaps futuros por prioridade

## P1 - Delegates e handlers de primeira classe

Impacto no Borealis:

- `borealis.game.events` idealmente deveria expor `event_on`, `event_off`, `event_once` e dispatch com handlers reais.
- hoje a versao mais segura e simples ficou baseada em fila (`emit`, `peek`, `poll`, `clear_pending`).

O que falta na linguagem:

- referencias de funcao seguras;
- callbacks tipados;
- talvez closures simples ou handlers sem captura como primeiro passo.

Beneficio direto:

- eventos de gameplay, UI e editor ficam muito mais naturais.
- reduz a necessidade de polling manual por frame.

## P1 - Tipo de valor generico para configuracoes e payloads

Impacto no Borealis:

- `settings_get(name) -> value` ainda nao ficou tao elegante quanto a decision original.
- hoje usamos `get_text`, `get_number` e `get_flag` porque ainda e o caminho mais claro e seguro.

O que falta na linguagem:

- enum/tagged union ergonomico para valores pequenos;
- ou um tipo `value`/`variant` padrao com `text`, `int`, `float`, `bool` e talvez `list/map`.

Beneficio direto:

- API de settings fica mais curta;
- payload de eventos e services fica mais expressivo;
- editor futuro ganha uma base melhor para inspetor generico.

## P1 - Serializacao estruturada e reflexao leve

Impacto no Borealis:

- `save`, `settings`, `scene`, `editor` e `storage` ainda dependem de escolhas manuais de formato e mapeamento.
- persistencia de estado grande continua trabalhosa para o dev.

O que falta na linguagem:

- serializacao derivada ou helper oficial para `struct`;
- reflexao leve de campos;
- opcionalmente anotacoes simples para incluir, renomear ou ignorar campo.

Beneficio direto:

- autosave, snapshot e configuracoes ficam muito mais faceis;
- editor futuro consegue inspecionar dados com menos codigo repetido.

## P1 - Dependencias de package locais mais explicitas

Impacto no Borealis:

- fixtures e exemplos externos ao package ainda nao tem um fluxo tao limpo para consumir `borealis` como dependencia local.
- isso faz alguns testes preferirem copia local de modulo em vez de importar o package diretamente.

O que falta no toolchain:

- dependencia local declarativa no `zenith.ztproj`;
- ou um fluxo padrao de workspace packages antes do `zpm`.

Beneficio direto:

- testes ficam mais proximos do uso real;
- exemplos externos ficam mais simples;
- reduz duplicacao entre package e fixtures.

## P2 - Hardening continuo do backend C para generics ricos

Impacto no Borealis:

- varios bloqueios ja foram resolvidos nesta fase;
- mesmo assim, Borealis continua sendo um bom estresse para `optional`, `map`, tipos qualificados e payloads gerenciados.

O que ainda vale perseguir:

- ampliar cobertura de testes para combinacoes aninhadas;
- manter paridade entre checker, lowering e emitter;
- reduzir cantos especiais no emitter C.

Beneficio direto:

- menos surpresa ao usar API modular e tipos mais ricos;
- mais confianca para crescer `scene`, `entities`, `save` e `services`.

## P2 - Reatribuicao segura de struct com campos gerenciados

Impacto no Borealis:

- padroes naturais como `prefs = settings.set_language(prefs, "pt-BR")` ainda podem gerar C fragil quando a `struct` tem campos como `text`, `list` ou `map`;
- hoje o package funciona melhor quando esse fluxo usa variaveis intermediarias em vez de varias reatribuicoes seguidas na mesma variavel.

O que falta no backend C:

- ordenar melhor `release` e `assign` em reatribuicao de `struct` com campos gerenciados;
- evitar que o valor antigo seja liberado cedo demais antes da proxima chamada usar o argumento atual;
- ampliar testes de regressao para cadeias de atualizacao de estado.

Beneficio direto:

- API de configuracoes, snapshots e editor state fica mais natural de usar;
- reduz workarounds em exemplos, fixtures e codigo de usuario.

## P2 - Acesso robusto a campos de struct em expressoes complexas

Impacto no Borealis:

- em `services` e `database`, o backend C ainda ficou sensivel a usos como `state.provider` ou `config.driver` dentro de alguns caminhos de retorno, `fmt`, `map_set` e construcao de valores;
- hoje o package fica mais estavel quando materializamos esses campos em `const` antes de reutilizar.

O que falta no backend C:

- aceitar melhor `get_field` em parametros de `struct` sem exigir tantas variaveis intermediarias;
- garantir emissao correta quando o campo participa de `success(...)`, `fmt`, `map_set` ou construcao de outra `struct`;
- ampliar fixtures de regressao para esse padrao.

Beneficio direto:

- reduz ruido no codigo da lib;
- deixa exemplos e APIs mais naturais para o usuario final.

## P2 - Try-propagate mais flexivel em chamadas diretas

Impacto no Borealis:

- em alguns pontos do fixture, `expr()?` dentro de `if` ou comparacao direta ainda exigiu materializacao previa em uma variavel local;
- o comportamento esta correto, mas a ergonomia ainda nao ficou tao boa quanto queremos.

O que falta no backend C:

- suportar melhor `?` em chamadas diretas usadas dentro de expressoes maiores;
- reduzir a necessidade de transformar `foo()? == 1` em `const value = foo()?`.

Beneficio direto:

- codigo mais limpo e mais proximo do estilo natural da linguagem;
- menos workarounds em testes, exemplos e app code.

## P2 - Ergonomia melhor para colecoes mutaveis

Impacto no Borealis:

- modulos como `events`, `assets`, `save` e `storage` ainda usam alguns sentinelas e loops manuais.

O que falta na linguagem/std:

- operacoes simples como `remove` real em `map`;
- push/pop/queue mais ergonomicos para `list`;
- helpers pequenos para iteracao e filtragem.

Beneficio direto:

- codigo da lib fica menor e mais legivel;
- menos truques de implementacao para representar fila, cache e indice.

## P2 - Linker e backend declarativos no manifesto

Impacto no Borealis:

- hoje o caminho desktop real ainda depende de profile manual e conhecimento de backend/linker.

O que falta no toolchain:

- declarar backend/linker profile no manifesto com menos configuracao manual;
- permitir fallback oficial mais claro por target.

Beneficio direto:

- onboarding melhor;
- futuro `zpm` mais previsivel;
- desktop native fica menos fragil para quem esta comecando.

## P3 - Metadados e anotacoes para editor futuro

Impacto no Borealis:

- editor-ready architecture ja esta desenhada;
- mas faltam mecanismos mais confortaveis para ligar dados de runtime a ferramentas.

O que falta na linguagem:

- anotacoes simples e estaveis para editor;
- IDs e metadata de campo com semantica clara;
- talvez uma camada de introspeccao opt-in.

Beneficio direto:

- facilita o futuro editor sem inventar uma segunda linguagem;
- aproxima o runtime real do modelo visual do Borealis Flow.

## O que este arquivo nao significa

- nao significa parar o Borealis ate tudo isso existir;
- nao significa mover logica de package para dentro do compilador;
- nao significa que toda melhoria aqui e bloqueio imediato.

Significa apenas:

- continuamos implementando o Borealis primeiro;
- quando um limite for mesmo estrutural, ele ja esta mapeado aqui com contexto.
