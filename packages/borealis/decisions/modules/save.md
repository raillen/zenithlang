# Borealis Module Decision - Save

- Status: accepted
- Date: 2026-04-23
- Type: module / persistence
- Scope: `borealis.game.save`

## Summary

`borealis.game.save` e a camada simples de progresso do jogo.

Ela cuida de:

- slots de save;
- autosave;
- snapshots com alias explicito de restauracao;
- campos de snapshot por slot (persistencia escalavel por elemento);
- leitura segura com `optional<text>`;
- reset local do estado em memoria durante testes e prototipos.

## Direcao de implementacao

- API curta e facil de entender;
- sem confundir string vazia com save apagado;
- estado separado entre `valor salvo` e `slot presente`;
- separar `snapshot inteiro` de `campos por snapshot`;
- preparada para no futuro trocar o backend em memoria por arquivo/JSON sem quebrar a API.

## Estado atual da implementacao

Hoje o modulo usa duas estruturas internas:

- `save_slots: map<int, text>` guarda o texto do slot;
- `save_present: map<int, bool>` guarda se o slot existe de verdade.
- `save_slot_fields: map<text, text>` guarda campos nomeados por slot;
- `save_slot_field_present: map<text, bool>` guarda presenca real dos campos;
- `save_slot_field_counts: map<int, int>` guarda contagem de campos por slot.

Isso evita um problema comum:

- `""` pode ser um valor valido;
- `slot apagado` precisa continuar sendo `none`.
- `campo apagado` tambem precisa continuar sendo `none`.

## API atual

- `slot_write(slot, snapshot) -> result<void, core.Error>`: grava o snapshot em um slot.
- `slot_read(slot) -> optional<text>`: le o snapshot do slot. Retorna `none` se nao existir.
- `slot_delete(slot) -> result<void, core.Error>`: marca o slot como apagado.
- `slot_exists(slot) -> bool`: informa se o slot existe.
- `autosave_write(snapshot) -> result<void, core.Error>`: grava no slot reservado de autosave.
- `autosave_read() -> optional<text>`: le o autosave atual.
- `autosave_delete() -> result<void, core.Error>`: apaga o autosave.
- `snapshot_write(slot, snapshot) -> result<void, core.Error>`: alias semantico de snapshot para `slot_write`.
- `snapshot_read(slot) -> optional<text>`: alias semantico para leitura de snapshot.
- `snapshot_restore(slot) -> optional<text>`: alias semantico de restauracao.
- `snapshot_delete(slot) -> result<void, core.Error>`: alias semantico para apagar snapshot.
- `slot_field_write(slot, name, value) -> result<void, core.Error>`: grava um campo nomeado dentro de um slot.
- `slot_field_read(slot, name) -> optional<text>`: le um campo nomeado. Retorna `none` se nao existir.
- `slot_field_delete(slot, name) -> result<void, core.Error>`: apaga um campo nomeado.
- `slot_field_exists(slot, name) -> bool`: informa se um campo nomeado existe.
- `slot_field_count(slot) -> int`: retorna quantos campos ativos o slot possui.
- `slot_field_clear(slot) -> result<void, core.Error>`: remove todos os campos ativos do slot.
- `autosave_field_write(name, value) -> result<void, core.Error>`: grava campo no autosave.
- `autosave_field_read(name) -> optional<text>`: le campo do autosave.
- `autosave_field_delete(name) -> result<void, core.Error>`: apaga campo do autosave.
- `autosave_field_exists(name) -> bool`: informa se campo do autosave existe.
- `autosave_field_count() -> int`: conta campos ativos do autosave.
- `autosave_field_clear() -> result<void, core.Error>`: remove todos os campos do autosave.
- `reset() -> void`: limpa todo o estado interno do modulo.

## Regras de comportamento

- slot negativo gera erro;
- nome de campo vazio gera erro;
- slot apagado volta `none`;
- campo apagado volta `none`;
- string vazia ainda pode ser um snapshot valido;
- string vazia ainda pode ser um campo valido;
- `autosave_*` usa o slot `0` por enquanto;
- `reset()` existe para testes, scaffolds e prototipos locais.

## Proximos passos desejados

- snapshots estruturados de entidades, cenas e mundo;
- integracao opcional com `storage`;
- formato persistente em JSON;
- adaptadores de serializacao por tipo no futuro.
