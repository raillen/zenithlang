# Borealis Module Decision - Services

- Status: accepted
- Date: 2026-04-23
- Type: module / network and remote services
- Scope: `borealis.game.services`

## Summary

`borealis.game.services` e o modulo guarda-chuva para rede e recursos remotos.

Nesta fase ele ainda e um stub em memoria, mas ja organiza tres ideias importantes:

- requisicoes HTTP simples;
- conexao de cloud save;
- upload, download e remocao de dados remotos de forma previsivel.

## Direcao de implementacao

- manter a API simples para iniciantes;
- separar rede de `save` e `storage`;
- permitir que o modulo cresca depois para APIs, multiplayer simples e cloud real;
- garantir um comportamento util mesmo antes do backend remoto existir.

## Estado atual da implementacao

Hoje o modulo funciona como stub deterministico:

- `http_send` valida URL e devolve uma resposta previsivel;
- cloud save usa armazenamento em memoria por `provider::name`;
- o estado de cloud separado entre `valor` e `presenca` evita confundir `""` com item removido.

## API atual

- `http_send(request) -> result<HttpResponse, core.Error>`: envia uma requisicao HTTP stub.
- `cloud_connect(provider) -> CloudState`: cria um estado conectado para um provider.
- `cloud_disconnect(state) -> CloudState`: desconecta o estado atual.
- `cloud_is_connected(state) -> bool`: informa se a conexao esta ativa.
- `cloud_upload(state, name, data) -> result<void, core.Error>`: envia um item para o provider atual.
- `cloud_download(state, name) -> result<optional<text>, core.Error>`: baixa um item. Retorna `none` se nao existir.
- `cloud_remove(state, name) -> result<void, core.Error>`: remove um item remoto.
- `cloud_exists(state, name) -> result<bool, core.Error>`: informa se o item remoto existe.
- `cloud_entry_count(state) -> result<int, core.Error>`: informa quantos itens o provider atual tem no stub.
- `request_count() -> int`: retorna quantas requisicoes HTTP stub foram feitas.
- `last_url() -> text`: retorna a ultima URL usada em `http_send`.
- `reset() -> void`: limpa todo o estado interno do modulo.

## Regras de comportamento

- URL vazia gera erro em `http_send`;
- metodo vazio cai para `GET`;
- provider vazio nao conecta;
- operacoes de cloud em estado desconectado retornam erro;
- item remoto removido volta `none`;
- string vazia ainda e um valor remoto valido;
- `reset()` existe para testes, exemplos e scaffolds.

## Proximos passos desejados

- separar subareas como `services.http`, `services.cloud` e `services.session`;
- integrar com `std.net` e requests reais;
- suportar sessoes remotas e multiplayer simples;
- permitir cloud save compartilhado com `save` e `storage`.
