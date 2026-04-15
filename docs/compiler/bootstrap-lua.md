# Bootstrap Compiler (Lua)

> Status: current as the active Lua toolchain reference, not as the official product front door.
> Papel atual: implementacao operacional da trilha Lua ativa para parser, binder, lowering, runtime e codegen.
> Front door oficial do produto: `ztc.lua` em caminho self-hosted estrito.

O Bootstrap Compiler em Lua nao e mais a face oficial do produto. A leitura correta hoje e mais estreita: ele continua sendo a implementacao de referencia da trilha Lua ativa e da maior parte da bateria de parser, binder, lowering e codegen, enquanto o entrypoint oficial do produto ficou self-hosted.

## O papel dele hoje

- referencia operacional da trilha Lua ativa;
- base da bateria principal de parser, binder, semantica e codegen;
- backend real que ainda sustenta runtime e execucao dos programas compilados;
- contraponto tecnico para comparar e endurecer a linha self-hosted.

## O que ele nao e mais

- nao e a identidade oficial do produto;
- nao e mais a narrativa principal de onboarding do compilador;
- nao substitui os docs correntes do estado self-hosted.

## Onde ele mora

- `ztc.lua` como front door que orquestra o caminho oficial;
- `src/syntax/`
- `src/semantic/`
- `src/lowering/`
- `src/backend/lua/`

## Relacao com Ascension

Ascension deixou de ser apenas trilha de paridade. Hoje ele define a face oficial do compilador no produto. O bootstrap em Lua continua importante como implementacao operacional da trilha ativa e como base do runtime/backend Lua, mas nao disputa mais o papel de front door oficial.

## Leitura complementar

- `ascension-zenith.md`
- `../language/current.md`
- `../specification/current-core.md`
- `../roadmap/selfhost-100.md`
- `../roadmap/selfhost-pos100.md`
