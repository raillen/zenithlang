# Especificacao: Core Atual Estabilizado

Atualizado em: 2026-04-15

Este documento resume o contrato curto do core do Zenith que hoje esta protegido por testes e pela trilha ativa canonica.

## 1. Caminhos canonicos

- compilador ativo oficial: `ztc.lua`
- compilador self-hosted canonico: `src/compiler/syntax.zt`
- `src/compiler/syntax_bridge.zt` e apenas stub legado parseavel

## 2. Regras semanticas estaveis

- `Optional<T>` e `Outcome<T, E>` sao a superficie idiomatica de ausencia e falha
- uso direto de `null` gera `ZT-S106`
- `T = null` ainda acumula `ZT-S100`
- listas e texto sao 1-based
- `or` desce para fallback no runtime atual
- UFCS virtual continua parte da surface real da linguagem

## 3. Garantias operacionais

- parser, binder, lowering, diagnostics e codegen da trilha ativa estao validados por bateria automatizada
- `src/compiler/syntax.zt` passa em `check`
- bootstrap self-hosted `stage2/stage3` e deterministico

## 4. O que nao e contrato de linguagem

- quantidade exata de `native lua` remanescente
- shape interno das ASTs self-hosted
- helpers temporarios como `empty_value()` e `value_is_present()`
- detalhes editoriais ou historicos do `syntax_bridge.zt`

## 5. Leitura complementar

- `types.md`
- `semantics.md`
- `transpilation.md`
- `../roadmap_estabilizacao.md`
