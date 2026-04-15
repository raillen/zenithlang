# Theme System V2

## Objetivo

Reimplementar o sistema de temas da Zenith IDE com uma abordagem em camadas, cobrindo:

- schema semantico mais rico para UI, editor, tabs, listas, estados e terminal
- builder visual em tres lentes de autoria: Quick, Regions e Advanced
- preview ao vivo entre janelas sem persistir mudancas de rascunho por engano
- importacao e exportacao de temas externos em JSON

## Arquivos principais

- `src/themes/index.ts`
- `src/utils/themeEngine.ts`
- `src/hooks/useThemeSync.ts`
- `src/components/ThemeLab.tsx`
- `src/App.tsx`
- `src/index.css`
- `src/components/MainLayout.tsx`
- `src/components/StatusBar.tsx`
- `src/components/InspectorPanel.tsx`
- `src/components/TabManager.tsx`
- `src/components/FileNavigator.tsx`

## Schema de tema

O schema foi expandido para ir alem do bloco antigo de `ui` e `syntax`.

### Tokens novos incorporados agora

#### Chrome
- `chrome.toolbarBackground`
- `chrome.sidebarBackground`
- `chrome.inspectorBackground`
- `chrome.statusBarBackground`

#### Editor
- `editor.activeLineBackground`
- `editor.activeLineBorder`
- `editor.gutterActiveForeground`

#### Tabs e listas
- `tab.activeBackground`
- `tab.inactiveBackground`
- `list.activeBackground`

#### States
Cada estado agora usa um tom completo:
- `foreground`
- `background`
- `border`

Estados atuais:
- `state.success`
- `state.warning`
- `state.danger`
- `state.info`

#### Terminal
A paleta do terminal segue declarativa:
- `background`
- `foreground`
- `cursor`
- `cursorAccent`
- `selectionBackground`
- ANSI base
- ANSI bright

## Lentes de autoria

O Theme Lab agora trabalha com tres niveis de edicao.

### 1. Quick
Pensado para quem quer montar um tema rapido por regiao, sem entrar no schema inteiro.

Grupos atuais:
- chrome
- editor
- navigation
- state surfaces
- terminal essentials

Essa lente usa os helpers:
- `getQuickTokens(theme)`
- `applyQuickTokens(theme, overrides)`

### 2. Regions
Pensado para abrir mais controle por area da IDE.

Grupos atuais:
- states completos
- syntax highlight
- terminal completo

Essa lente usa:
- `getRegionTokens(theme)`
- `applyRegionTokens(theme, overrides)`

### 3. Advanced
Pensado para autoria completa do tema semantico.

Grupos atuais:
- metadados
- UI base
- surface
- border
- text
- accent
- interaction

## Preview ao vivo entre janelas

O preview do Theme Lab precisava funcionar como ferramenta separada, sem salvar rascunho como se fosse configuracao final.

### Solucao implementada

- `src/hooks/useThemeSync.ts` agora sincroniza temas por evento com payload contendo:
  - `theme`
  - `id`
  - `source`
- cada janela ignora eventos emitidos por ela mesma
- a janela principal escuta os previews e aplica o tema em memoria via store + `applyThemeToDOM`
- o `ThemeLab` transmite o rascunho com o id fixo `__theme_lab_preview__`
- ao desmontar o lab, o tema salvo anteriormente e restaurado localmente e reenviado para a janela principal

Resultado:
- preview ao vivo funciona
- o rascunho nao vira persistencia acidental
- salvar continua sendo uma acao explicita via `Aplicar tema`

## Theme Lab

O `ThemeLab` foi refeito como workspace de construcao, e nao mais como painel apertado de configuracoes.

### Estrutura atual
- coluna esquerda: preset, identidade do tema, lente ativa e acoes de import/export
- coluna central: editor da lente atual
- coluna direita: preview do workbench e orientacao do fluxo

### Fluxo atual
1. escolher preset base
2. editar em Quick, Regions ou Advanced
3. validar no preview vivo
4. salvar com `Aplicar tema`
5. exportar para `.zenith-theme.json` se quiser compartilhar

## Integracao com a IDE

Os tokens novos ja foram ligados em pontos visuais importantes:

- toolbar
- sidebar esquerda
- inspector
- status bar
- tabs
- selecao do file navigator
- Monaco editor
- terminal

## Compatibilidade

A leitura de tema continua aceitando formatos antigos quando possivel.

A funcao `parseThemeObject` faz normalizacao e fallback para:
- tokens ausentes
- estados antigos declarados como string simples
- temas customizados persistidos antes do schema atual

## Formato externo

O formato externo priorizado agora e JSON.

Extensao recomendada:
- `.zenith-theme.json`

Motivos:
- validacao simples
- import/export direto no Theme Lab
- portabilidade
- facilidade de versionamento
- previsibilidade para syntax highlight e terminal

## Validacao executada

Comandos validados nesta etapa:

- `npm run build`

Status:
- build concluido com sucesso
- warning conhecido do Vite sobre chunk grande permanece, sem bloquear a entrega

## Proximos passos recomendados

1. adicionar mais tokens de editor
   - selecao inativa
   - find match
   - word highlight
   - gutter default

2. adicionar familias semanticas para diagnostics e git
   - error / warning / info / hint
   - added / modified / deleted / renamed

3. mapear mais componentes da IDE para tokens semanticos
   - problems panel
   - command palette
   - settings dialog
   - Zenith Hub

4. documentar o contrato publico do arquivo `.zenith-theme.json`

5. adicionar comparacao lado a lado no Theme Lab
   - tema salvo
   - rascunho atual

## Resumo

A reimplementacao saiu de um modelo de troca de paleta para um sistema de temas mais solido, com:

- schema semantico ampliado
- builder em camadas
- preview entre janelas
- persistencia explicita
- base pronta para evolucao de temas externos e documentacao publica
