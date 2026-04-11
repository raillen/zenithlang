# Ascension Compiler (Zenith)

> Status: `Current`
> Implementação oficial hoje: `src/compiler/`
> Recomendado para: build real, contribuição principal e arquitetura nativa.

Ascension e o motor auto-hospedado oficial do Zenith. Hoje ele compila Zenith com Zenith, aplica otimizações reais ao próprio pipeline e sustenta a distribuição para plataformas modernas.

## O que Ascension entrega hoje

- self-hosting completo
- parsing, binding, lowering, diagnostics e codegen no motor nativo
- otimizações como constant folding e inlining
- CLI unificada para build, run e testes
- FFI nativo para integração com C e C++
- distribuição multiplataforma com alvos como `@windows`, `@linux` e `@macos`
- modos standalone e bundle para empacotamento

## Onde ele mora no repositorio

Os pontos principais dessa linha estao em:

- `src/compiler/`
- `src/compiler/syntax.zt`
- `src/compiler/syntax_bridge.zt`
- `tests/ascension/`

## O que mudou de status

Ascension j? não existe como incubacao ou promessa técnica. Em `2026-04-11`, eu passei a trata-lo como o motor concluído da linguagem e como a linha oficial do Zenith atual.

## O papel do bootstrap agora

Com Ascension concluído, o bootstrap em Lua continua importante, mas em outro lugar:

- como registro técnico da evolução
- como referência histórica e comparativa
- como material ?til para estudo e manutenção do legado