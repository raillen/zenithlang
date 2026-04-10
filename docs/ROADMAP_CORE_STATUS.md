# Status Atual do ROADMAP_CORE

Documento derivado da leitura de [docs/ROADMAP_CORE.md](docs/ROADMAP_CORE.md) cruzada com o estado real do core da linguagem em 2026-04-09.

## Resumo

- As Fases 1, 2 e 3 est├úo substancialmente conclu├¡das no core da linguagem.
- O Zenith agora possui interoperabilidade direta com Lua e um sistema de itera├º├úo extens├¡vel.
- A pipeline de tipos foi unificada para fun├º├╡es e lambdas.

## Atualizacao 2026-04-09

- **P0/P1 Conclu├¡das**: Sistema de tipos ADT, Operator `?`, Match/Where e Async/Await est├íveis.
- **F3. Blocos `native lua`**: Implementados como statement e express├úo, permitindo escape direto para Lua.
- **F3. Trait `Iterable`**: O loop `for-in` agora ├й extens├¡vel via Trait `Iterable<T>` na Prelude.
- **Binder**: Suporte a gen├йricos em Traits e bloco `apply` agora instanciam corretamente os par├вmetros.
- **Type System**: Unifica├º├úo de tipos de fun├º├úo e lambdas com verifica├º├úo estrutural (covari├вncia de retorno).

## Matriz de Status (v1.0-alpha)

| Item | Status | Observa├º├úo | Prioridade |
| --- | --- | --- | --- |
| F1. Lambdas | **Conclu├¡do** | Integrados ao sistema de tipos `FUNC` | P2 |
| F1. Destrutura├º├úo em par├вmetros | **Conclu├¡do** | Suportado via lowering de padr├╡es | P2 |
| F1. Spread em express├╡es | **Conclu├¡do** | `..lista` e `{..mapa}` implementados | P2 |
| F1. Type aliases avancados | **Conclu├¡do** | `type`/`union` gen├йricos funcionais | P1 |
| F2. Enums com dados | **Conclu├¡do** | Sum types gen├йricos integrados | P0 |
| F2. Generic constraints profundas | **Conclu├¡do** | Valida├º├úo de membros via Traits funcional | P1 |
| F2. Slicing de colecoes | **Conclu├¡do** | Suporte nativo `obj[i..j]` funcional | P2 |
| F2. Option/Result types | **Conclu├¡do** | `Optional` e `Outcome` na prelude com `?` | P0 |
| F3. Deep desugaring na IR | **Conclu├¡do** | Match/Where baixam via Lowerer | P1 |
| F3. Transparent async/await | **Conclu├¡do** | Pipeline est├ível com corrotinas | P1 |
| F3. Blocos `native lua` | **Conclu├¡do** | Implementado no parser, binder e codegen | P2 |
| F3. Trait `Iterable` | **Conclu├¡do** | Loop `for-in` desacoplado e extens├¡vel | P2 |
| F4. Stdlib | **Concluído** | `core`, `io`, `math`, `os`, `fs`, `json` OK | P2 |
| F4. ZPM / `.ztproj` | Parcial | Suporte a projetos local; falta Git/HTTP | P2 |
| F4. Auditoria de estabilidade | **Concluído** | Core v0.2.5 estabilizado com testes integrados | P1 |

## Próximos Passos (v0.3.0 - Utilidades)

1. Implementar `std.time` (Sleep/Timestamp API).
2. Implementar `std.text` (Split/Join/Regex).
3. Evoluir o ZPM para download de pacotes via Git/HTTP.
4. Expandir suite de testes para auditoria de 1.0.

---
*Atualizado em: 09 de Abril de 2026*
