# Zenith Local RAG Starter

Este starter cria um assistente local para o repositório Zenith.

Ele faz 3 coisas:

1. lê partes importantes do projeto;
2. quebra em pedaços menores;
3. busca os pedaços certos antes de perguntar ao modelo.

Resultado: o modelo para de responder "no escuro" e passa a usar a spec, as decisions, o compilador, o runtime e os testes como contexto.

## O que este starter indexa

Por padrão:

- `README.md`
- `language/`
- `compiler/`
- `runtime/c/`
- `stdlib/`
- `examples/`
- `tests/`

Ele ignora ruído comum, como:

- `dist/`
- `reports/`
- `target/`
- builds temporários
- fuzz gerado
- binários

## Stack

- `Ollama` ou `LM Studio` para embeddings e resposta
- índice vetorial local simples em `.rag/`
- scripts Python simples em `tools/rag/`

## Arquivos

- `tools/rag/ingest.py`: cria ou atualiza o índice
- `tools/rag/chat.py`: faz perguntas ao índice
- `tools/rag/chunking.py`: quebra docs e código em chunks
- `tools/rag/common.py`: configuração, paths e clientes dos provedores

## Setup rápido no Windows

1. Crie uma venv:

```powershell
python -m venv .venv
.venv\Scripts\activate
```

2. Instale a dependência local:

```powershell
pip install -r tools/rag/requirements.txt
```

## Fluxo recomendado com LM Studio

1. No LM Studio, baixe:

- um modelo de chat para código
- um modelo de embedding

Sugestão inicial:

- chat: um `Qwen2.5-Coder 3B Instruct` em GGUF
- embedding: `nomic-embed-text-v1.5`

2. No LM Studio:

- abra `Developer`
- ligue `Start server`
- confirme a base URL local

Use este valor como padrão:

```text
http://localhost:1234/v1
```

3. Descubra os identificadores dos modelos no LM Studio.

Você vai precisar de:

- `id` do modelo de chat
- `id` do modelo de embedding

4. Gere o índice usando LM Studio:

```powershell
python tools/rag/ingest.py --provider lmstudio --base-url http://localhost:1234/v1 --embedding-model "nomic-embed-text-v1.5" --rebuild
```

5. Faça a primeira pergunta:

```powershell
python tools/rag/chat.py --provider lmstudio --base-url http://localhost:1234/v1 --embedding-model "nomic-embed-text-v1.5" --model "qwen2.5-coder-3b-instruct" "Como o runtime trata ownership e copy-on-write?"
```

Observações importantes:

- no LM Studio, o nome real do modelo pode mudar conforme o arquivo GGUF baixado;
- se o nome acima não funcionar, use o identificador exato mostrado pelo LM Studio;
- o tamanho de contexto normalmente é configurado no carregamento do modelo dentro do LM Studio.

## Fluxo com Ollama

3. Baixe os modelos no Ollama:

```powershell
ollama pull qwen2.5-coder:3b
ollama pull embeddinggemma
```

Para o Granite, use o nome exato que aparecer no seu `ollama list` ou no catálogo do Ollama.
No seu ambiente atual, o nome instalado é `granite-code:3b-instruct`.

4. Gere o índice:

```powershell
python tools/rag/ingest.py
```

5. Faça a primeira pergunta:

```powershell
python tools/rag/chat.py --model qwen2.5-coder:3b "Como o runtime trata ownership e copy-on-write?"
```

## Trocar entre Qwen e Granite

### No Ollama

Qwen:

```powershell
python tools/rag/chat.py --model qwen2.5-coder:3b "Onde o emitter C trata strings?"
```

Granite:

```powershell
python tools/rag/chat.py --model granite-code:3b-instruct "Onde o emitter C trata strings?"
```

### No LM Studio

Use os identificadores exatos carregados no servidor local:

```powershell
python tools/rag/chat.py --provider lmstudio --base-url http://localhost:1234/v1 --embedding-model "nomic-embed-text-v1.5" --model "<id-do-granite-no-lm-studio>" "Onde o emitter C trata strings?"
```

## Comandos úteis

Rebuild completo:

```powershell
python tools/rag/ingest.py --provider lmstudio --base-url http://localhost:1234/v1 --embedding-model "nomic-embed-text-v1.5" --rebuild
```

Indexar só uma parte para teste rápido:

```powershell
python tools/rag/ingest.py --provider lmstudio --base-url http://localhost:1234/v1 --embedding-model "nomic-embed-text-v1.5" --roots language runtime/c --limit 20
```

Abrir shell interativo:

```powershell
python tools/rag/chat.py --provider lmstudio --base-url http://localhost:1234/v1 --embedding-model "nomic-embed-text-v1.5" --model "qwen2.5-coder-3b-instruct"
```

Ver o contexto recuperado:

```powershell
python tools/rag/chat.py --provider lmstudio --base-url http://localhost:1234/v1 --embedding-model "nomic-embed-text-v1.5" --model "qwen2.5-coder-3b-instruct" --show-context "Como Zenith define diagnostics?"
```

## Onde os dados ficam

- metadados do índice: `.rag/zenith_store_<embedding>.json`
- vetores: `.rag/zenith_vectors_<embedding>.npy`
- manifesto local: `.rag/zenith_manifest_<embedding>.json`

Pode apagar `.rag/` quando quiser reconstruir do zero.

## Como o ranking funciona

O starter dá prioridade maior para:

1. `language/spec/`
2. `language/decisions/`
3. `compiler/` e `runtime/c/`
4. `stdlib/`
5. `examples/` e `tests/`

Isso ajuda o modelo a preferir a spec antes de confiar em teste ou exemplo isolado.

## Quando reindexar

Reindexe quando mudar:

- spec
- decisions
- compilador
- runtime
- stdlib
- exemplos
- testes importantes

Na maior parte do tempo, basta rodar:

```powershell
python tools/rag/ingest.py
```

O script é incremental. Ele reprocessa só arquivos novos, alterados ou removidos.

## Próximos upgrades

Quando esta base estiver funcionando bem, os próximos passos mais úteis são:

- criar um conjunto pequeno de perguntas benchmark;
- adicionar filtros por `kind` (`spec`, `decision`, `code`, `test`);
- testar outro embedding;
- depois pensar em `QLoRA` para ajustar estilo e formato de resposta.
