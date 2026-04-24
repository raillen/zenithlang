# Borealis Module Decision - Assets

- Status: accepted
- Date: 2026-04-23
- Type: module / asset loading
- Scope: `borealis.game.assets`

## Summary

`borealis.game.assets` cuida do ciclo de vida de recursos como imagens, texto e binarios.

Nesta fase, ele ainda e um cache em memoria, mas ja ganhou um comportamento mais util:

- diferencia asset conhecido de asset atualmente carregado;
- preserva `id` estavel mesmo depois de unload e reload;
- oferece loaders especificos para os tipos mais comuns da camada 2D;
- oferece consultas simples por chave, tipo e `id`;
- expoe metadata leve para editor e tooling, como `source`, `created_stamp`, `loaded_stamp` e `reload_count`.

## Direcao de implementacao

- manter loading e lookup simples;
- nao misturar regra de gameplay aqui;
- expor handles claros e previsiveis;
- preparar o modulo para integrar backend real depois.

## Estado atual da implementacao

Hoje o modulo:

- carrega `image`, `texture`, `text`, `binary`, `font`, `sound`, `music`, `shader`, `atlas` e `spritesheet`;
- guarda metadata por `id`;
- preserva o registro do asset mesmo quando ele e descarregado;
- permite recarga sem criar um novo `id`;
- oferece unload e reload por handle, chave ou `id`;
- protege a chave simples contra ambiguidade de tipo.

Isso ajuda bastante em:

- editor futuro;
- hot reload simples;
- debug de assets;
- sistemas que guardam referencia por `id`.

## API atual

- `asset_load(path, kind) -> result<AssetHandle, core.Error>`: carrega ou recarrega um asset pelo tipo.
- `asset_load_from(path, kind, source = "runtime") -> result<AssetHandle, core.Error>`: carrega com origem logica explicita para cache, tooling e editor.
- `asset_load_image(path)`: atalho para `kind = "image"`.
- `asset_load_texture(path)`: atalho para `kind = "texture"`.
- `asset_load_text(path)`: atalho para `kind = "text"`.
- `asset_load_binary(path)`: atalho para `kind = "binary"`.
- `asset_load_font(path)`: atalho para `kind = "font"`.
- `asset_load_sound(path)`: atalho para `kind = "sound"`.
- `asset_load_music(path)`: atalho para `kind = "music"`.
- `asset_load_shader(path)`: atalho para `kind = "shader"`.
- `asset_load_atlas(path)`: atalho para `kind = "atlas"`.
- `asset_load_spritesheet(path)`: atalho para `kind = "spritesheet"`.
- `asset_get(key) -> optional<AssetHandle>`: retorna o asset apenas se estiver carregado.
- `asset_get_kind(kind, key) -> optional<AssetHandle>`: lookup carregado por tipo + chave.
- `asset_find(key) -> optional<AssetHandle>`: retorna o asset mesmo se ele estiver descarregado.
- `asset_find_kind(kind, key) -> optional<AssetHandle>`: lookup por tipo + chave, incluindo descarregado.
- `asset_find_id(id) -> optional<AssetHandle>`: lookup por `id`.
- `asset_exists_kind(kind, key) -> bool`: informa se uma chave daquele tipo ja foi registrada.
- `asset_info(asset) -> optional<AssetInfo>`: retorna metadata do asset pelo handle.
- `asset_info_key(key) -> optional<AssetInfo>`: retorna metadata pela chave.
- `asset_info_id(id) -> optional<AssetInfo>`: retorna metadata pelo `id`.
- `asset_unload(asset) -> result<void, core.Error>`: descarrega o asset, mas preserva o registro.
- `asset_unload_key(key) -> result<void, core.Error>`: descarrega por chave simples.
- `asset_unload_id(id) -> result<void, core.Error>`: descarrega por `id`.
- `asset_reload(asset) -> result<AssetHandle, core.Error>`: recarrega um asset conhecido usando o mesmo `id`.
- `asset_reload_key(key) -> result<AssetHandle, core.Error>`: recarrega por chave simples.
- `asset_reload_id(id) -> result<AssetHandle, core.Error>`: recarrega por `id`.
- `asset_cache_clear() -> result<void, core.Error>`: limpa todo o cache e o registro.
- `asset_count() -> int`: conta assets atualmente carregados.
- `asset_total_count() -> int`: conta assets registrados no cache.
- `asset_count_kind(kind) -> int`: conta quantos assets carregados daquele tipo existem.
- `asset_total_count_kind(kind) -> int`: conta quantos assets registrados daquele tipo existem.
- `asset_count_source(source) -> int`: conta quantos assets carregados vieram daquela origem logica.
- `asset_total_count_source(source) -> int`: conta quantos assets registrados vieram daquela origem logica.
- `asset_exists(key) -> bool`: informa se a chave ja foi registrada.
- `asset_is_loaded(asset) -> bool`: informa se o handle esta carregado.
- `asset_is_loaded_key(key) -> bool`: informa se a chave esta carregada.

## Regras de comportamento

- caminho vazio gera erro;
- tipo vazio gera erro;
- a chave simples precisa continuar apontando para um unico tipo; registrar o mesmo caminho com outro `kind` gera erro claro;
- `asset_get(...)` devolve `none` se o asset estiver descarregado;
- `asset_find(...)` ainda encontra o asset descarregado;
- `asset_reload(...)` restaura o handle com o mesmo `id`;
- `created_stamp` e `loaded_stamp` sao stamps logicos monotonicamente crescentes, nao horario do sistema;
- `reload_count` comeca em `0` e cresce a cada recarga;
- `asset_cache_clear()` zera o estado inteiro do modulo.

## Proximos passos desejados

- integrar carregamento real por backend (filesystem + runtime nativo);
- incluir metadata opcional de tamanho e hash para invalidacao;
- adicionar hooks de hot reload para editor/tooling;
- avaliar estrategia de budget/memoria por tipo de asset.
