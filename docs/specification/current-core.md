# Especificacao: Core Atual Estabilizado

Atualizado em: 2026-04-20

Este documento resume o contrato curto do core do Zenith que hoje esta protegido por testes, bootstrap, gate self-hosted de release e politica explicita de artefatos.

## 1. Caminhos canonicos

- front door oficial do produto: `ztc.lua` em modo self-hosted estrito;
- compilador self-hosted canonico: `src/compiler/syntax.zt`;
- artefato promovido de bootstrap/release: `ztc_selfhost.lua`;
- `src/compiler/syntax_bridge.zt` e apenas stub legado parseavel;
- `src/semantic/binding/binder.zt`, `src/syntax/parser/parser.zt` e `src/syntax/lexer/lexer.zt` nao sao caminho oficial; permanecem como superficie experimental ate promocao explicita.

## 2. Regras semanticas estaveis

- `Optional<T>` e `Outcome<T, E>` sao a superficie idiomatica de ausencia e falha;
- uso direto de `null` gera `ZT-S106`;
- `T = null` ainda acumula `ZT-S100`;
- listas e texto sao 1-based;
- `or` desce para fallback no runtime atual;
- UFCS virtual continua parte da surface real da linguagem.

## 3. Garantias operacionais

- parser, binder, lowering, diagnostics e codegen da trilha ativa estao validados por bateria automatizada;
- `src/compiler/syntax.zt` passa em `check` pela frente self-hosted oficial;
- bootstrap self-hosted `stage2/stage3` e deterministico;
- `lua tools/selfhost_release.lua` valida a promocao recorrente de release no recorte oficial;
- artefatos temporarios do fluxo oficial vivem em `.selfhost-artifacts/` e `.ztc-tmp/`.

## 4. O que nao e contrato de linguagem

- shape interno das ASTs self-hosted;
- detalhes internos do artefato promovido `ztc_selfhost.lua`;
- helpers fora da ABI nomeada em `selfhost-abi.md`;
- detalhes editoriais ou historicos do `syntax_bridge.zt`.

## 5. Leitura complementar

- `types.md`
- `semantics.md`
- `transpilation.md`
- `selfhost-abi.md`
- `selfhost-artifacts.md`
- `../roadmap/selfhost-100.md`
- `../roadmap/selfhost-pos100.md`
- `../roadmap/MASTER.md`
- `decisions/002-selfhost-architecture-cutover.md`
- `decisions/004-selfhost-100-cutover.md`