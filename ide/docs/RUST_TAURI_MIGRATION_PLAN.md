# Rust/Tauri Migration Plan

Este documento registra os principais candidatos de migracao de logica e infraestrutura da Zenith IDE para Rust/Tauri com foco em performance, escalabilidade e reducao de overhead entre frontend e backend.

## Objetivo

Melhorar a responsividade da IDE em workspaces medios e grandes sem mover UI desnecessariamente para o backend.

O foco principal nao e "reescrever React em Rust", e sim:

- reduzir scans completos do filesystem
- reduzir polling e recomputacao no frontend
- reduzir round-trips pela bridge Tauri
- transformar jobs pesados em servicos persistentes com cache, streaming e invalidacao

## Principio Arquitetural

### React deve continuar dono de:

- renderizacao da interface
- estado visual local
- composicao de paineis, dialogs e layout
- integracao com Monaco e xterm no nivel de view

### Rust/Tauri deve assumir:

- indexacao e busca do workspace
- sincronizacao incremental do file tree
- pipeline de diagnosticos
- jobs de compilacao e execucao
- persistencia nativa de settings e session state
- canais de eventos de alta frequencia

## Prioridades de Migracao

## 1. Busca global e quick open

### Estado atual

- `search_in_files` faz varredura completa do workspace em cada busca:
  - `src-tauri/src/lib.rs`
- `search_file_names` repete outra varredura completa a cada digitacao:
  - `src-tauri/src/lib.rs`
- O frontend consulta esses comandos em:
  - `src/components/GlobalSearch.tsx`
  - `src/components/CommandPalette.tsx`

### Problemas atuais

- leitura completa e sincronica de muitos arquivos
- nenhuma indexacao em memoria
- nenhum cache
- nenhum cancelamento de busca obsoleta
- nenhum streaming incremental de resultados

### Migracao recomendada

Criar um `WorkspaceIndexService` em Rust com:

- indice de nomes de arquivos
- indice de conteudo textual
- atualizacao por watcher de filesystem
- filtros compartilhados entre quick open e search in files
- retorno incremental via eventos Tauri

### Ganho esperado

Muito alto. Esta e a area com maior chance de gargalo real em projetos grandes.

## 2. Pipeline de diagnosticos durante a digitacao

### Estado atual

- o debounce e controlado no frontend em:
  - `src/components/EditorPanel.tsx`
- o backend roda `lua ztc.lua` por chamada em:
  - `src-tauri/src/lib.rs`
- o resultado e agregado no frontend em:
  - `src/store/useWorkspaceStore.ts`
  - `src/components/ProblemsPanel.tsx`
  - `src/components/MainLayout.tsx`

### Problemas atuais

- um processo de compilador por execucao
- arquivo temporario fixo `.check.zt`
- risco de corrida em chamadas concorrentes
- parser de erro fragil baseado em regex textual
- possibilidade de resultados chegarem fora de ordem

### Migracao recomendada

Criar um `DiagnosticsService` em Rust com:

- fila de jobs por documento
- cancelamento de jobs antigos
- cache por arquivo e hash de conteudo
- arquivos temporarios unicos ou comunicacao por stdin
- payload estruturado para diagnosticos
- emissao de eventos somente quando o resultado ainda for valido

### Ganho esperado

Muito alto durante edicao continua.

## 3. Workspace model e file navigator incremental

### Estado atual

- o frontend faz polling de arvore e git status em:
  - `src/components/FileNavigator.tsx`
- o backend monta a arvore inteira recursivamente em:
  - `src-tauri/src/lib.rs`

### Problemas atuais

- custo alto para abrir workspaces grandes
- substituicao completa da arvore no frontend
- rerender em cascata
- polling periodico em vez de invalidacao incremental

### Migracao recomendada

Criar um `WorkspaceStateService` em Rust com:

- lazy loading por diretorio
- cache da estrutura do workspace
- watcher para arquivos e pastas
- atualizacoes incrementais para a UI
- integracao com status de git por arquivo visivel

### Ganho esperado

Alto no startup, na troca de projeto e na navegacao em repositorios grandes.

## 4. Jobs de compilacao e execucao

### Estado atual

- `run_compiler` dispara `lua` sob demanda e retorna a saida inteira ao final:
  - `src-tauri/src/lib.rs`

### Problemas atuais

- sem streaming de stdout/stderr
- sem cancelamento
- sem reaproveitamento de processo
- sem fila de jobs

### Migracao recomendada

Criar um `CompilerJobService` em Rust com:

- jobs de `check`, `build` e `run`
- streaming de saida por eventos
- cancelamento de jobs em andamento
- metadados de status e duracao
- opcao futura de processo persistente

### Ganho esperado

Medio a alto, principalmente se o compilador for chamado com frequencia.

## 5. Terminal bridge e I/O de alta frequencia

### Estado atual

- o PTY ja esta no backend:
  - `src-tauri/src/lib.rs`
- o frontend ainda envia input por `invoke` a cada entrada:
  - `src/components/TerminalPanel.tsx`

### Problemas atuais

- overhead de bridge em sessoes intensas
- eventos pequenos e frequentes
- ausencia de batching/coalescing
- ausencia de backpressure explicito

### Migracao recomendada

Evoluir o terminal atual para:

- bufferizacao curta de eventos
- batching de input e output
- politicas de flush por tamanho e tempo
- metrica simples de throughput

### Ganho esperado

Medio. Vale mais a pena se o terminal for parte central do fluxo de trabalho.

## 6. Persistencia nativa de settings e session state

### Estado atual

- a store acessa SQLite diretamente no frontend:
  - `src/store/useWorkspaceStore.ts`

### Problemas atuais

- acoplamento entre estado visual e persistencia
- escritas repetidas por chave
- mais round-trips do que o necessario

### Migracao recomendada

Criar comandos Rust como:

- `load_settings`
- `save_settings`
- `load_session_state`
- `save_session_state`

Com:

- cache em memoria
- escrita em lote
- formato estruturado e versionado

### Ganho esperado

Medio. Menos por CPU e mais por organizacao, consistencia e menor chatter entre camadas.

## 7. Git status incremental

### Estado atual

- o backend chama `git status --porcelain`:
  - `src-tauri/src/lib.rs`

### Problemas atuais

- custo crescente em repositorios grandes
- recalculo completo sem cache

### Migracao recomendada

Substituir por uma estrategia incremental:

- cache de status
- invalidacao por watcher
- opcao de integrar `git2`
- escopo limitado aos caminhos visiveis quando possivel

### Ganho esperado

Medio.

## Nao priorizar para migracao por performance

Estas partes nao devem ser alvo inicial:

- theming e aplicacao de CSS vars
- componentes visuais simples
- dropdowns e filtros pequenos no frontend
- logica local de tabs, preview e dirty state que nao faz I/O pesado

Arquivos representativos:

- `src/utils/themeEngine.ts`
- `src/components/ui/SearchableDropdown.tsx`
- `src/components/SettingsDialog.tsx`

## Ordem sugerida de execucao

1. Busca global + quick open com indexacao em memoria
2. Diagnosticos com worker persistente
3. File tree incremental com watcher
4. Compiler jobs com streaming e cancelamento
5. Terminal batching
6. Persistencia nativa de settings/session
7. Git status incremental

## Arquitetura alvo

## Backend Rust/Tauri

Servicos sugeridos:

- `WorkspaceIndexService`
- `WorkspaceStateService`
- `DiagnosticsService`
- `CompilerJobService`
- `SettingsService`
- `TerminalTransportService`

Cada servico deve expor:

- comandos sincronos curtos para bootstrap
- eventos para atualizacoes incrementais
- cache proprio
- invalidacao controlada

## Frontend React

O frontend deve:

- fazer bootstrap inicial do estado
- ouvir eventos do backend
- atualizar stores locais com patches pequenos
- renderizar views, listas e paineis

O frontend nao deve:

- fazer polling onde watcher resolve melhor
- disparar scans completos por digitacao
- controlar filas pesadas de jobs

## Fases recomendadas

## Fase 1

- substituir `search_in_files` e `search_file_names` por indice em memoria
- adicionar watcher do workspace
- preparar API incremental para resultados de busca

### Detalhamento tecnico da Fase 1

#### Objetivo operacional

Trocar o modelo atual de busca "varrer disco a cada consulta" por um servico residente em Rust que:

- constroi um indice inicial do workspace
- mantem esse indice atualizado com watcher
- atende quick open e busca textual sobre o mesmo estado em memoria
- entrega resultados incrementais ao frontend
- permite cancelamento de buscas obsoletas

#### Escopo da Fase 1

Entram nesta fase:

- indexacao de nomes de arquivo
- indexacao basica de conteudo textual
- watcher de filesystem
- comandos Tauri para bootstrap e gerenciamento de buscas
- eventos Tauri para progresso, resultados parciais e invalidacao

Nao entram nesta fase:

- ranking sofisticado por fuzzy score avancado
- busca semantica
- git status incremental
- diagnosticos
- lazy tree do navigator

## Arquitetura alvo da Fase 1

### Backend Rust

Criar um `WorkspaceIndexService` responsavel por:

- conhecer o `workspace_root`
- escanear arquivos elegiveis
- manter metadados dos arquivos indexados
- atualizar o indice quando houver criacao, alteracao ou remocao
- responder consultas de `quick open`
- responder consultas de `search in files`
- emitir eventos incrementais para a UI

### Frontend React

O frontend deve:

- pedir bootstrap do indice ao abrir ou trocar de workspace
- ouvir eventos de progresso do indice
- disparar consultas sem forcar scans completos
- cancelar consultas antigas quando o usuario continuar digitando
- renderizar resultados incrementais e estado de carregamento

O frontend nao deve:

- continuar usando polling para quick open
- reexecutar busca completa a cada tecla via scan de disco
- assumir que todos os resultados chegam de uma vez

## Estruturas Rust sugeridas

### Estado principal

```rust
use std::collections::HashMap;
use std::path::PathBuf;
use std::sync::atomic::{AtomicU64, Ordering};
use std::sync::{Arc, RwLock};

type SearchToken = u64;

#[derive(Clone, Debug, serde::Serialize, serde::Deserialize)]
#[serde(rename_all = "camelCase")]
pub struct IndexedFile {
    pub path: String,
    pub relative_path: String,
    pub file_name: String,
    pub extension: Option<String>,
    pub size_bytes: u64,
    pub modified_unix_ms: u128,
    pub text_hash: u64,
    pub is_text: bool,
}

#[derive(Default)]
pub struct WorkspaceIndexState {
    pub workspace_root: RwLock<PathBuf>,
    pub files: RwLock<Vec<IndexedFile>>,
    pub content_cache: RwLock<HashMap<String, Arc<str>>>,
    pub active_searches: RwLock<HashMap<SearchToken, SearchControl>>,
    pub next_search_token: AtomicU64,
}

pub struct SearchControl {
    pub cancelled: Arc<std::sync::atomic::AtomicBool>,
}
```

### Eventos e payloads

```rust
#[derive(Clone, Debug, serde::Serialize)]
#[serde(rename_all = "camelCase")]
pub struct IndexBootstrapStarted {
    pub workspace_root: String,
}

#[derive(Clone, Debug, serde::Serialize)]
#[serde(rename_all = "camelCase")]
pub struct IndexBootstrapProgress {
    pub scanned_files: usize,
    pub indexed_files: usize,
}

#[derive(Clone, Debug, serde::Serialize)]
#[serde(rename_all = "camelCase")]
pub struct IndexBootstrapCompleted {
    pub total_files: usize,
    pub total_text_files: usize,
}

#[derive(Clone, Debug, serde::Serialize)]
#[serde(rename_all = "camelCase")]
pub struct FileNameHit {
    pub name: String,
    pub path: String,
}

#[derive(Clone, Debug, serde::Serialize)]
#[serde(rename_all = "camelCase")]
pub struct SearchMatch {
    pub line_number: usize,
    pub line_content: String,
}

#[derive(Clone, Debug, serde::Serialize)]
#[serde(rename_all = "camelCase")]
pub struct FileContentHit {
    pub file_path: String,
    pub matches: Vec<SearchMatch>,
}

#[derive(Clone, Debug, serde::Serialize)]
#[serde(rename_all = "camelCase")]
pub struct SearchResultsChunk {
    pub token: SearchToken,
    pub query: String,
    pub mode: String,
    pub done: bool,
    pub items: Vec<SearchChunkItem>,
}

#[derive(Clone, Debug, serde::Serialize)]
#[serde(tag = "kind", rename_all = "camelCase")]
pub enum SearchChunkItem {
    FileName(FileNameHit),
    FileContent(FileContentHit),
}
```

### Configuracao de indexacao

```rust
pub struct IndexConfig {
    pub ignored_dirs: Vec<&'static str>,
    pub ignored_extensions: Vec<&'static str>,
    pub max_file_size_bytes: u64,
    pub max_preview_line_len: usize,
}
```

Valores iniciais sugeridos:

- ignorar `.git`, `node_modules`, `target`, `dist`, `.tauri`
- ignorar arquivos binarios obvios como `.png`, `.jpg`, `.ico`, `.woff2`, `.db`
- limitar indexacao textual inicial a arquivos menores que `1_000_000` bytes

## Comandos Tauri sugeridos

### 1. Bootstrap do indice

```rust
#[tauri::command]
fn workspace_index_bootstrap(root_path: String) -> Result<WorkspaceIndexSnapshot, String>
```

Responsabilidades:

- resolver o `root_path`
- limpar indice anterior
- iniciar scan inicial
- registrar watcher do workspace
- devolver um snapshot resumido para o frontend

Payload de retorno sugerido:

```rust
#[derive(serde::Serialize)]
#[serde(rename_all = "camelCase")]
pub struct WorkspaceIndexSnapshot {
    pub workspace_root: String,
    pub total_files: usize,
    pub total_text_files: usize,
    pub is_ready: bool,
}
```

### 2. Consulta de quick open

```rust
#[tauri::command]
fn workspace_query_files(query: String, limit: Option<usize>) -> Result<Vec<FileNameHit>, String>
```

Regras:

- operar somente sobre o indice em memoria
- retornar ordenado por score simples
- aplicar `limit` padrao de 100

### 3. Inicio de busca textual incremental

```rust
#[tauri::command]
fn workspace_search_start(
    query: String,
    is_regex: bool,
    match_case: bool,
    max_results: Option<usize>,
) -> Result<SearchToken, String>
```

Responsabilidades:

- gerar um token unico
- registrar busca ativa
- rodar a busca em thread separada
- emitir chunks de resultados enquanto processa

### 4. Cancelamento de busca

```rust
#[tauri::command]
fn workspace_search_cancel(token: SearchToken) -> Result<(), String>
```

Responsabilidades:

- marcar a busca como cancelada
- impedir envio de novos resultados para esse token

### 5. Estado resumido do indice

```rust
#[tauri::command]
fn workspace_index_status() -> Result<WorkspaceIndexSnapshot, String>
```

Uso:

- mostrar se o indice ja terminou bootstrap
- depurar readiness no frontend

## Eventos Tauri sugeridos

Padrao de nomes:

- `zenith://workspace-index-started`
- `zenith://workspace-index-progress`
- `zenith://workspace-index-completed`
- `zenith://workspace-index-invalidated`
- `zenith://workspace-search-chunk`
- `zenith://workspace-search-completed`
- `zenith://workspace-search-error`

### Contrato de eventos

#### `zenith://workspace-index-started`

Emitido quando um novo workspace comeca a ser indexado.

Uso no frontend:

- mostrar estado de warming/indexing
- limpar resultados anteriores

#### `zenith://workspace-index-progress`

Emitido durante o scan inicial.

Uso no frontend:

- barra ou texto de progresso
- telemetria simples de bootstrap

#### `zenith://workspace-index-completed`

Emitido quando o indice inicial termina.

Uso no frontend:

- remover loading global de quick open e search
- habilitar mensagens mais confiaveis de "indice pronto"

#### `zenith://workspace-index-invalidated`

Emitido quando o watcher detecta mudancas relevantes.

Payload minimo sugerido:

```rust
#[derive(serde::Serialize)]
#[serde(rename_all = "camelCase")]
pub struct IndexInvalidationEvent {
    pub path: String,
    pub kind: String,
}
```

Uso no frontend:

- opcionalmente mostrar feedback de reindexacao
- invalidar listas que dependem do estado anterior

#### `zenith://workspace-search-chunk`

Emitido varias vezes durante uma busca textual longa.

Uso no frontend:

- append incremental de resultados
- evitar congelamento visual em repositorios grandes

#### `zenith://workspace-search-completed`

Emitido no fim da busca do token atual.

Uso no frontend:

- finalizar loading
- impedir spinner infinito

#### `zenith://workspace-search-error`

Emitido se a busca falhar por regex invalida ou erro interno.

Uso no frontend:

- renderizar erro inline no painel de busca

## Fluxo sugerido de quick open

1. app chama `workspace_index_bootstrap` ao abrir ou trocar de workspace
2. backend constroi indice inicial e liga watcher
3. command palette chama `workspace_query_files` a cada digitacao com debounce curto
4. consulta usa apenas memoria, sem scan de disco
5. frontend renderiza top resultados imediatamente

## Fluxo sugerido de busca global

1. usuario digita e aciona busca
2. frontend cancela o token anterior se existir
3. frontend chama `workspace_search_start`
4. backend emite `workspace-search-chunk` com lotes pequenos
5. frontend agrega os chunks por token
6. backend emite `workspace-search-completed`
7. frontend fecha loading do token atual

## Estrategia de implementacao no backend

### Primeira iteracao

- manter indice simples em memoria
- usar `walkdir` ou `ignore` para scan
- usar `notify` para watcher
- usar `Arc<RwLock<...>>` para estado
- usar `std::thread::spawn` para jobs de busca

### Segunda iteracao dentro da mesma fase

- adicionar score melhor para quick open
- adicionar truncamento de linhas grandes
- evitar reler arquivos nao alterados
- medir custo de memoria do cache de conteudo

## Ajustes esperados no frontend

### `src/components/CommandPalette.tsx`

Trocar:

- uso direto de `search_file_names`

Por:

- `workspace_query_files`

### `src/components/GlobalSearch.tsx`

Trocar:

- chamada unica de `search_in_files`

Por:

- `workspace_search_start`
- listener de `workspace-search-chunk`
- listener de `workspace-search-completed`
- cancelamento de token anterior

### `src/App.tsx` ou bootstrap global

Adicionar:

- bootstrap do indice ao iniciar app
- rebootstrap ao trocar `currentProjectRoot`
- listener de progresso se quiser indicador global

## Criterios de conclusao da Fase 1

- quick open nao faz mais scan de disco por tecla
- busca global usa resultados incrementais
- existe cancelamento de buscas obsoletas
- o indice reage a mudancas de arquivo sem rebuild completo a cada consulta
- o frontend nao depende mais dos comandos antigos `search_in_files` e `search_file_names`

## Riscos especificos da Fase 1

- uso excessivo de memoria se o cache textual crescer sem limites
- watcher gerar eventos em excesso e provocar reindexacao redundante
- concorrencia mal controlada entre bootstrap e buscas ativas
- frontend misturar resultados de tokens diferentes

## Mitigacoes

- limitar tamanho de arquivo indexado
- aplicar debounce/coalescing nos eventos do watcher
- usar token por busca e ignorar chunks antigos no frontend
- comecar com indice simples e medir antes de sofisticar scoring

## Fase 2

- reestruturar diagnosticos como worker persistente
- cancelar jobs obsoletos
- entregar payload estruturado para erros

## Fase 3

- trocar file tree full scan por modelo incremental
- remover polling de `FileNavigator`
- integrar git status incremental

## Fase 4

- criar jobs de compilacao com streaming
- melhorar terminal com batching
- mover persistencia de settings para backend

## Riscos e cuidados

- evitar mover estado puramente visual para Rust
- evitar criar APIs backend excessivamente finas, que mantem muito chatter
- versionar payloads de eventos para facilitar evolucao futura
- medir antes e depois de cada fase

## Metricas de sucesso

- tempo para abrir workspace
- tempo para quick open responder
- tempo para busca global retornar primeiros resultados
- latencia de diagnosticos apos digitacao
- numero de round-trips frontend/backend por fluxo
- numero de rerenders em listas grandes

## Resumo executivo

O maior ganho de performance neste projeto nao esta em migrar mais interface para Rust. O melhor retorno vem de consolidar em Rust os fluxos que hoje ainda funcionam em modo "scan everything", "spawn process" e "polling". A migracao deve focar em indexacao, watchers, cache, streaming e jobs persistentes.
