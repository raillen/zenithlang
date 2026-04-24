# Borealis Module Decision - Settings

- Status: accepted
- Date: 2026-04-23
- Type: module / configuration
- Scope: `borealis.game.settings`

## Summary

`borealis.game.settings` cuida das preferencias do jogador e das configuracoes de experiencia.

Nesta fase, o modulo cobre:

- audio basico;
- idioma;
- acessibilidade;
- preferencias simples de video;
- persistencia por perfil em cima de `storage`.

## Direcao de implementacao

- manter a API curta e previsivel;
- separar claramente `defaults`, estado atual e perfis salvos;
- preservar o uso amigavel para iniciantes;
- permitir persistencia sem depender de banco de dados ou sistema externo.

## Estado atual da implementacao

Hoje o modulo:

- tem um tipo central `Settings`;
- guarda um `current_state` global simples;
- oferece setters claros para audio, video e acessibilidade;
- normaliza valores fora do intervalo aceito;
- salva e carrega perfis nomeados usando `storage`;
- aceita leitura por nome com `get_text`, `get_number` e `get_flag`.

## Entidades e campos principais

- `Settings.master_volume`: volume master de `0.0` a `1.0`.
- `Settings.music_volume`: volume de musica de `0.0` a `1.0`.
- `Settings.sfx_volume`: volume de efeitos de `0.0` a `1.0`.
- `Settings.language`: idioma atual, como `pt-BR`.
- `Settings.accessibility_dyslexia_font`: liga fonte amigavel para dislexia.
- `Settings.accessibility_high_contrast`: liga contraste alto.
- `Settings.accessibility_reduce_motion`: reduz movimento visual.
- `Settings.video_fullscreen`: liga fullscreen.
- `Settings.video_vsync`: liga `vsync`.
- `Settings.video_render_scale`: escala de render entre `0.5` e `2.0`.
- `Settings.ui_scale`: escala de interface entre `0.75` e `2.0`.

## API atual

- `defaults() -> Settings`: devolve o estado padrao.
- `current_state() -> Settings`: devolve o estado global atual.
- `apply_defaults() -> Settings`: restaura defaults no estado global.
- `apply_state(next) -> Settings`: aplica um estado no global.
- `set_language(current, language) -> Settings`: troca idioma.
- `set_master_volume(current, volume) -> Settings`: ajusta volume master.
- `set_music_volume(current, volume) -> Settings`: ajusta volume de musica.
- `set_sfx_volume(current, volume) -> Settings`: ajusta volume de efeitos.
- `set_dyslexia_font(current, enabled) -> Settings`: liga ou desliga fonte amigavel para dislexia.
- `set_high_contrast(current, enabled) -> Settings`: liga ou desliga contraste alto.
- `set_reduce_motion(current, enabled) -> Settings`: liga ou desliga reducao de movimento.
- `set_fullscreen(current, enabled) -> Settings`: liga ou desliga fullscreen.
- `set_vsync(current, enabled) -> Settings`: liga ou desliga `vsync`.
- `set_render_scale(current, scale) -> Settings`: ajusta escala de render.
- `set_ui_scale(current, scale) -> Settings`: ajusta escala da interface.
- `save_profile(profile, value) -> result<void, core.Error>`: salva um perfil nomeado em `storage`.
- `load_profile(profile) -> optional<Settings>`: carrega um perfil nomeado; devolve `none` se ele nao existir ou estiver invalido.
- `apply_profile(profile) -> optional<Settings>`: carrega o perfil e aplica no estado global.
- `delete_profile(profile) -> result<void, core.Error>`: remove um perfil salvo.
- `profile_exists(profile) -> bool`: informa se o perfil existe.
- `reset(current, name) -> Settings`: reseta uma chave para o default.
- `reset_all() -> Settings`: devolve o estado padrao inteiro.
- `list_names() -> list<text>`: lista os nomes aceitos pelas APIs de leitura/reset.
- `get_text(current, name) -> optional<text>`: le um campo textual.
- `get_number(current, name) -> optional<float>`: le um campo numerico.
- `get_flag(current, name) -> optional<bool>`: le um campo booleano.

## Regras de comportamento

- idioma vazio volta para o idioma padrao;
- volumes sempre sao clampados entre `0.0` e `1.0`;
- `video_render_scale` fica entre `0.5` e `2.0`;
- `ui_scale` fica entre `0.75` e `2.0`;
- perfil vazio gera erro;
- perfil inexistente devolve `none` em `load_profile` e `apply_profile`;
- `save_profile` sobrescreve o perfil se ele ja existir;
- perfis sao persistidos em `storage` com chaves previsiveis;
- flags salvas aceitam leitura de `true/false`, `1/0`, `yes/no` e `on/off`.

## Proximos passos desejados

- separar melhor `settings.video` e `settings.audio` quando o package crescer;
- adicionar resolucao, modo janela e presets de qualidade;
- permitir exportacao/importacao em blob textual;
- integrar com menu de opcoes da camada `ui`.
