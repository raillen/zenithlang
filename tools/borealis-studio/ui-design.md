Abaixo está uma especificação em **Markdown** completa, detalhada e orientada para desenvolvimento da interface **CityScene Engine**, baseada nas imagens fornecidas.

````markdown
# CityScene Engine — Especificação Completa da Interface

## 1. Visão Geral

O **CityScene Engine** é uma interface web inspirada em editores profissionais de desenvolvimento 3D, especialmente no padrão visual e funcional de engines como Unity, Unreal e ferramentas DCC modernas.

A interface deve permitir ao usuário:

- Visualizar e manipular uma cena 3D urbana.
- Navegar por uma hierarquia de objetos da cena.
- Selecionar objetos e editar suas propriedades.
- Gerenciar assets do projeto.
- Alternar entre visualização de cena e jogo.
- Executar, pausar e avançar a simulação.
- Acessar ferramentas de colaboração, conta, camadas e layout.
- Visualizar status de build, avisos e erros.

A aplicação deve ter aparência **limpa, técnica, moderna e produtiva**, com foco em clareza visual, alta densidade de informação e comportamento responsivo.

---

# 2. Estrutura Geral da Tela

## 2.1 Resolução Base

A interface deve ser projetada inicialmente para:

- **Resolução base:** 1920 × 1080
- **Aspect ratio:** 16:9
- **Layout:** desktop-first
- **Estilo:** aplicação web full-screen
- **Altura mínima recomendada:** 800 px
- **Largura mínima recomendada:** 1280 px

---

## 2.2 Divisão Principal

A tela é dividida em cinco grandes regiões:

1. **Header Global**
2. **Toolbar lateral esquerda**
3. **Painel de Hierarquia**
4. **Viewport central da cena**
5. **Inspector lateral direito**
6. **Painel inferior de projeto**
7. **Status bar inferior**

Representação simplificada:

```text
┌──────────────────────────────────────────────────────────────────────────────┐
│                                HEADER GLOBAL                                │
├──────┬────────────────────┬──────────────────────────────────┬──────────────┤
│ TOOL │     HIERARCHY      │          SCENE VIEWPORT          │  INSPECTOR   │
│ BAR  │                    │                                  │              │
│      │                    │                                  │              │
├──────┴────────────────────┴──────────────────────────────────┴──────────────┤
│                              PROJECT / CONSOLE / ANIMATION                  │
├──────────────────────────────────────────────────────────────────────────────┤
│                                  STATUS BAR                                  │
└──────────────────────────────────────────────────────────────────────────────┘
````

---

# 3. Header Global

## 3.1 Posição

O header ocupa toda a largura superior da tela.

```text
x: 0
y: 0
width: 100%
height: 48px
```

## 3.2 Estilo Visual

* Fundo: `#F7F8FA`
* Borda inferior: `1px solid #E6E8EC`
* Layout horizontal com alinhamento central vertical.
* Padding horizontal: `8px`
* Gap entre grupos: `12px`

---

## 3.3 Elementos do Header

### 3.3.1 Logo e Seletor de Projeto

Localizado no canto superior esquerdo.

```text
┌───────────────┐
│ ⬢ CityScene ▼ │
└───────────────┘
```

Especificação:

* Ícone cúbico azul à esquerda.
* Texto: `CityScene`
* Seta para baixo indicando dropdown.
* Altura: `36px`
* Padding horizontal: `10px`
* Border-radius: `6px`
* Hover: fundo `#EEF4FF`

Funcionalidade:

* Ao clicar, abre menu com:

  * Projetos recentes
  * Abrir projeto
  * Criar novo projeto
  * Configurações do projeto

---

### 3.3.2 Seletor de Branch / Configuração

Ao lado do seletor de projeto.

```text
┌──────────┐
│ main ▼   │
└──────────┘
```

Especificação:

* Ícone de branch/git à esquerda.
* Texto: `main`
* Dropdown.
* Altura: `36px`
* Largura aproximada: `96px`

Funcionalidade:

* Selecionar branch/configuração ativa.
* Exibir lista de branches.
* Exibir estado atual do projeto.

---

### 3.3.3 Botões Globais de Ação

Logo após o branch selector.

```text
┌────┐ ┌────┐ ┌────┐
│ 🔍 │ │ 🌐 │ │ ↗ │
└────┘ └────┘ └────┘
```

Botões:

1. Busca global
2. Serviços ou mundo/conectividade
3. Abrir em tela externa / exportar / compartilhar

Especificação:

* Tamanho: `36px × 36px`
* Border-radius: `6px`
* Borda: `1px solid #E0E3E7`
* Fundo: branco ou transparente
* Hover: `#F0F4F8`

---

### 3.3.4 Controles de Play

Centralizados no header.

```text
┌────┬────┬────┐
│ ▶  │ ⏸ │ ⏭ │
└────┴────┴────┘
```

Posição:

* Deve ficar visualmente centralizado na tela, não apenas no espaço restante.
* Grupo com três botões colados.

Especificação:

```text
height: 36px
button width: 44px
border: 1px solid #DCE1E7
background: #FFFFFF
```

Botões:

| Botão | Função               |
| ----- | -------------------- |
| Play  | Executa a simulação  |
| Pause | Pausa a simulação    |
| Step  | Avança um frame/tick |

Estados:

* Normal: branco
* Hover: `#F3F6FA`
* Ativo: `#E8F1FF`
* Play ativo: ícone azul `#2563EB`

---

### 3.3.5 Ações do Lado Direito

No canto superior direito:

```text
┌────────┐ ┌─────────┐ ┌────────┐ ┌────────┐ ┌──────┐
│ Colab  │ │ Account │ │ Layers │ │ Layout │ │ Avatar │
└────────┘ └─────────┘ └────────┘ └────────┘ └──────┘
```

#### Colab

* Botão com ícone de colaboração.
* Texto: `Colab`
* Abre painel ou menu de usuários conectados.

#### Account

* Ícone circular azul.
* Texto: `Account`
* Dropdown.
* Mostra opções de conta, login, workspace e billing.

#### Layers

* Dropdown para camadas visíveis/editáveis da cena.

#### Layout

* Dropdown para presets de layout da interface.

#### Avatar

* Imagem circular do usuário.
* Tamanho: `32px × 32px`
* Canto direito absoluto com margem `12px`.

---

# 4. Toolbar Lateral Esquerda

## 4.1 Posição

A toolbar fica encostada à lateral esquerda, abaixo do header.

```text
x: 0
y: 48px
width: 48px
height: calc(100vh - 48px - 28px)
```

## 4.2 Estilo

* Fundo: `#FFFFFF`
* Borda direita: `1px solid #E6E8EC`
* Ícones centralizados.
* Cada botão: `40px × 40px`
* Espaçamento vertical: `4px`
* Padding superior: `8px`

---

## 4.3 Ferramentas

Ordem vertical:

```text
┌────┐
│ 🖐 │ Selecionar / mão
├────┤
│ ◇  │ Mover
├────┤
│ ✣  │ Rotacionar
├────┤
│ ⛶  │ Escalar
├────┤
│ ⤢  │ Transform universal
└────┘
```

### Ferramenta Selecionada

Na imagem, a ferramenta ativa é a primeira.

Estilo ativo:

```css
background: #2F80ED;
color: #FFFFFF;
border-radius: 6px;
```

Demais ferramentas:

```css
color: #4B5563;
background: transparent;
```

Hover:

```css
background: #EEF4FF;
color: #2563EB;
```

---

# 5. Painel Hierarchy

## 5.1 Posição

Painel lateral esquerdo, imediatamente após a toolbar.

```text
x: 48px
y: 48px
width: 360px aproximadamente
height: calc(100vh - 48px - painel inferior - status bar)
```

Na composição visual, ocupa cerca de 18% da largura total.

---

## 5.2 Header do Painel

```text
┌──────────────────────────────┐
│  Hierarchy              🔒   │
└──────────────────────────────┘
```

Especificação:

* Altura: `40px`
* Texto: `Hierarchy`
* Ícone à esquerda indicando tipo de painel.
* Ícone de cadeado à direita.
* Borda inferior: `1px solid #E6E8EC`

---

## 5.3 Campo de Busca

Logo abaixo do header.

```text
┌──────────────────────────────┐
│ 🔍  All                      │
└──────────────────────────────┘
```

Especificação:

```text
height: 32px
margin: 8px
border-radius: 6px
background: #FFFFFF
border: 1px solid #E1E5EA
```

Funcionalidade:

* Filtra objetos da hierarquia.
* Placeholder: `All`
* Deve aceitar busca por nome de objeto, tipo ou componente.

---

## 5.4 Tree View da Cena

Estrutura exibida:

```text
▾ CityScene
  ◇ Main Camera
  ◇ Directional Light
  ◇ Sky and Fog Volume
  ▾ Environment
    ▾ Buildings
      ◇ SM_Building_A_01
      ◇ SM_Building_A_02
      ◇ SM_Building_B_01
      ◇ SM_Building_Corner_01
    ▾ Props
      ◇ SM_Street_Lamp_01
      ◇ SM_Bench_01
      ◇ SM_TrashCan_01
      ◇ SM_Planter_01
    ▾ Vehicles
      ◇ SM_Car_01
      ◇ SM_Car_02
      ◇ SM_Truck_01
  ▾ Gameplay
    ◇ PlayerStart
    ◇ Waypoints
  ◇ PostProcessing Volume
  ◇ Audio Listener
```

---

## 5.5 Item Selecionado

Objeto selecionado:

```text
SM_Building_A_01
```

Estilo:

```css
background: #E8F1FF;
color: #1F2937;
border-radius: 4px;
```

O item selecionado deve sincronizar com:

* Viewport
* Inspector
* Project panel
* Breadcrumbs
* Gizmo ativo

---

## 5.6 Comportamento da Hierarquia

Funcionalidades obrigatórias:

* Expandir/recolher grupos.
* Selecionar objeto.
* Drag-and-drop para reorganizar.
* Menu contextual no clique direito.
* Renomear com duplo clique ou tecla `F2`.
* Multiseleção com `Shift` e `Ctrl/Cmd`.
* Filtro por busca.
* Ícones diferentes por tipo:

  * Cena
  * Camera
  * Luz
  * Folder
  * Mesh
  * Volume
  * Audio

---

# 6. Viewport Principal

## 6.1 Posição

Área central principal da aplicação.

```text
x: 408px
y: 48px
width: restante entre Hierarchy e Inspector
height: até o painel inferior
```

O viewport é o elemento visual dominante da interface.

---

## 6.2 Tabs Superiores

No topo do viewport:

```text
┌────────┬────────┐
│ Scene  │ Game   │
└────────┴────────┘
```

### Tab Scene

* Ativa na imagem.
* Ícone de cubo ou cena.
* Texto: `Scene`.

### Tab Game

* Inativa.
* Ícone de gamepad ou tela.
* Texto: `Game`.

Estilo tab ativa:

```css
background: #FFFFFF;
border-bottom: 2px solid #2563EB;
color: #111827;
```

Tab inativa:

```css
background: #F7F8FA;
color: #6B7280;
```

---

## 6.3 Toolbar Superior do Viewport

Abaixo das tabs:

```text
┌───────┬────────┬────┬────┬────┬────┬──────────────┬────────┐
│ Pivot │ Global │ 2D │ ◎  │ ▱  │ ⛶ │   Gizmos ▼   │ Search │
└───────┴────────┴────┴────┴────┴────┴──────────────┴────────┘
```

### Elementos

| Elemento             | Função                        |
| -------------------- | ----------------------------- |
| Pivot                | Alterna origem de manipulação |
| Global               | Alterna espaço Global/Local   |
| 2D                   | Alterna modo 2D               |
| Ícone eixo/cubo      | Alterna visualização espacial |
| Ícone camadas        | Render/display layers         |
| Ícone mão/ferramenta | Opções de manipulação         |
| Gizmos               | Liga/desliga gizmos           |
| Search               | Busca dentro da cena          |

---

## 6.4 Viewport 3D

A região principal mostra uma cena urbana.

### Conteúdo visual esperado

A cena deve conter:

* Cruzamento urbano em perspectiva.
* Prédios de tijolo e concreto.
* Calçadas.
* Ruas com faixas de pedestre.
* Árvores nas laterais.
* Carros e caminhão.
* Poste de luz.
* Hidrante.
* Céu claro com nuvens.
* Skyline ao fundo.
* Iluminação diurna.
* Profundidade de cena com prédios distantes levemente suavizados.

---

## 6.5 Manipulador de Transformação

No centro da cena há um gizmo de transformação do objeto selecionado.

```text
       Y
       ↑
       │
       │
X ←────●────→ Z
```

Especificação visual:

* Eixo X: vermelho
* Eixo Y: verde
* Eixo Z: azul
* Setas 3D com hastes finas.
* Posicionado sobre o objeto selecionado.
* Deve responder a drag do mouse.

---

## 6.6 Gizmo de Orientação no Canto Superior Direito

No canto superior direito do viewport:

```text
       Y
       ↑
  X ←  ●  → Z
       Persp
```

Especificação:

* Mostra orientação dos eixos.
* Labels:

  * `X` vermelho
  * `Y` verde
  * `Z` azul
* Texto abaixo: `Persp`
* Deve permitir clicar nos eixos para alterar a câmera.

Funcionalidades:

* Clicar no eixo X: vista lateral.
* Clicar no eixo Y: vista superior.
* Clicar no eixo Z: vista frontal.
* Clicar no centro: resetar orientação.
* Clicar em `Persp`: alternar perspectiva/ortográfica.

---

## 6.7 Barra Vertical Interna de Ferramentas do Viewport

No lado esquerdo interno do viewport:

```text
┌────┐
│ 🖐 │
├────┤
│ ◇  │
├────┤
│ ◆  │
├────┤
│ ⤨  │
├────┤
│ ⛶  │
└────┘
```

Posição:

```text
x: 16px dentro do viewport
y: 80px abaixo do topo do viewport
width: 40px
```

Estilo:

* Fundo branco.
* Border-radius: `8px`
* Box-shadow leve.
* Ícones centralizados.
* Ferramenta ativa azul.

---

# 7. Inspector

## 7.1 Posição

Painel lateral direito.

```text
x: viewport end
y: 48px
width: 360px a 420px
height: calc(100vh - 48px - status bar)
```

Na imagem, ocupa cerca de 23% da largura.

---

## 7.2 Header do Inspector

```text
┌──────────────────────────────┐
│ Inspector                🔒  │
└──────────────────────────────┘
```

Especificação:

* Altura: `40px`
* Ícone à esquerda.
* Texto: `Inspector`
* Cadeado à direita.
* Borda inferior.

---

## 7.3 Cabeçalho do Objeto Selecionado

```text
☑  ◇  SM_Building_A_01              Static ☐
```

Elementos:

* Checkbox ativo indicando objeto habilitado.
* Ícone cúbico do objeto.
* Nome do objeto: `SM_Building_A_01`
* Checkbox `Static` no lado direito.

Funcionalidades:

* Checkbox principal ativa/desativa objeto.
* Nome editável.
* Static define se o objeto é estático para iluminação, bake, otimização etc.

---

## 7.4 Linha Tag e Layer

```text
Tag   [ Untagged ▼ ]      Layer   [ Default ▼ ]
```

Especificação:

* Dois dropdowns horizontais.
* Cada input com altura `32px`.
* Border-radius `6px`.
* Borda `#DDE2E8`.

Funcionalidades:

* Tag: categoriza objeto.
* Layer: define camada de renderização/interação.

---

# 8. Componentes do Inspector

Cada componente aparece como um painel/accordion.

## 8.1 Padrão Visual de Componentes

```text
▾ Component Name                         ⋮
──────────────────────────────────────────
conteúdo
```

Estilo:

* Header: `36px`
* Ícone à esquerda
* Nome em semibold
* Botão de opções no lado direito
* Accordion expansível/recolhível
* Borda inferior entre componentes
* Padding interno: `12px`

---

## 8.2 Transform

### Layout

```text
▾ Transform

Position    X [12.34]   Y [0.00]   Z [-8.43]
Rotation    X [0]       Y [90]     Z [0]
Scale       X [1]       Y [1]      Z [1]
```

### Campos

| Propriedade |     X |    Y |     Z |
| ----------- | ----: | ---: | ----: |
| Position    | 12.34 | 0.00 | -8.43 |
| Rotation    |     0 |   90 |     0 |
| Scale       |     1 |    1 |     1 |

Especificação dos inputs:

```css
height: 28px;
width: 64px;
border: 1px solid #DDE2E8;
border-radius: 5px;
background: #FFFFFF;
text-align: center;
font-size: 12px;
```

Funcionalidade:

* Inputs numéricos.
* Alteração em tempo real do objeto.
* Suporte a arrastar label para incrementar valores.
* Reset por menu contextual.
* Undo/redo integrado.

---

## 8.3 Mesh Filter

```text
▾ Mesh Filter

Mesh    [ SM_Building_A_01 ▼ ]
```

Função:

* Define o mesh usado pelo objeto.
* Dropdown com busca.
* Aceita drag-and-drop de asset do Project.

---

## 8.4 Mesh Renderer

```text
▾ Mesh Renderer

Materials                         [2]

Element 0    [ M_Building_Brick ▼ ]
Element 1    [ M_Window_Glass  ▼ ]
```

Função:

* Controla materiais do objeto.
* Lista materiais por slot.
* Número de materiais editável.

Campos:

* `Element 0`: material de tijolo.
* `Element 1`: material de vidro.

Funcionalidades:

* Drag-and-drop de materiais.
* Preview de material.
* Expandir array.
* Adicionar/remover elementos.
* Reordenar slots.

---

## 8.5 Lighting

```text
▾ Lighting

Cast Shadows                  [ On ▼ ]
Receive Shadows               [ ☑ ]
Contribute Global Illumination[ ☐ ]
Light Probes                  [ Blend Probes ▼ ]
```

Funcionalidades:

* Controlar sombras projetadas.
* Controlar recepção de sombras.
* Habilitar contribuição para iluminação global.
* Definir comportamento de light probes.

---

## 8.6 Probes

```text
▾ Probes

Light Probes          [ Blend Probes ▼ ]
Reflection Probes     [ Blend Probes ▼ ]
Anchor Override       [ None (Transform)  × ]
```

Função:

* Configura probes de iluminação e reflexão.
* Anchor Override aceita referência a Transform.

---

## 8.7 Additional Settings

```text
▾ Additional Settings

Motion Vectors       [ Per Object Motion ▼ ]
Dynamic Occlusion    [ ☑ ]
```

Função:

* Ajustes adicionais de renderização.
* Motion Vectors para pós-processamento/motion blur.
* Dynamic Occlusion para otimização.

---

## 8.8 Material Preview

Na parte inferior do Inspector:

```text
● M_Building_Brick (Material)

Shader   [ Standard ▼ ]          [ Edit... ]

[ Add Component ]
```

Elementos:

* Preview circular do material.
* Nome do material.
* Dropdown de shader.
* Botão `Edit...`
* Botão grande `Add Component`

Especificação do botão:

```css
height: 36px;
width: calc(100% - 32px);
border-radius: 6px;
border: 1px solid #DDE2E8;
background: #FFFFFF;
font-weight: 500;
```

---

# 9. Painel Inferior

## 9.1 Posição

Painel inferior abaixo do viewport e da hierarquia.

```text
x: 48px
y: viewport bottom
width: calc(100% - 48px - inspector width)
height: 260px aproximadamente
```

Na imagem, o painel ocupa cerca de 25% da altura.

---

## 9.2 Tabs do Painel Inferior

```text
┌─────────┬─────────┬───────────┐
│ Project │ Console │ Animation │
└─────────┴─────────┴───────────┘
```

### Project

Ativa na imagem.

### Console

Mostra logs, warnings e erros.

### Animation

Permite edição de animações.

Estilo:

* Tab ativa com fundo branco.
* Borda superior azul ou texto mais escuro.
* Tabs inativas com fundo `#F7F8FA`.

---

## 9.3 Layout Interno do Project

O painel Project é dividido em duas colunas:

```text
┌─────────────────────┬──────────────────────────────────────────────┐
│ Favorites / Folders │                 Asset Grid                   │
└─────────────────────┴──────────────────────────────────────────────┘
```

### Coluna Esquerda

Largura aproximada: `220px`.

Contém:

```text
Favorites
  All Materials
  All Models
  All Prefabs
  All Scripts

Assets
  Environment
  Props
  Vehicles
  Materials
  Textures
  Scripts
  Scenes
  Settings

Packages
```

Funcionalidades:

* Navegação por pastas.
* Expandir/recolher árvore.
* Seleção de pasta.
* Menu contextual.
* Criar nova pasta.
* Importar asset.

---

## 9.4 Breadcrumb

Acima da grade de assets:

```text
Assets > Environment > Buildings
```

Especificação:

* Altura: `32px`
* Texto pequeno, `12px`
* Separadores `>`
* Cada segmento clicável.

---

## 9.5 Grade de Assets

Conteúdo exibido:

```text
┌─────────────┐ ┌─────────────┐ ┌─────────────┐ ┌─────────────┐ ┌─────────────┐ ┌─────────────┐
│ thumbnail   │ │ thumbnail   │ │ thumbnail   │ │ thumbnail   │ │ thumbnail   │ │ folder      │
│ Building A1 │ │ Building A2 │ │ Building B1 │ │ Building B2 │ │ Corner 01   │ │ Materials   │
└─────────────┘ └─────────────┘ └─────────────┘ └─────────────┘ └─────────────┘ └─────────────┘
```

Assets visíveis:

1. `SM_Building_A_01`
2. `SM_Building_A_02`
3. `SM_Building_B_01`
4. `SM_Building_B_02`
5. `SM_Building_Corner_01`
6. `Materials`

### Thumbnail de Asset

Especificação:

```text
width: 140px
height thumbnail: 100px
label height: 24px
border-radius: 6px
```

Estilo:

* Thumbnail com fundo `#EEF1F5`
* Imagem centralizada do modelo 3D.
* Nome abaixo em `12px`.
* Hover: contorno azul claro.
* Selecionado: fundo `#E8F1FF`, borda `#2563EB`.

---

## 9.6 Barra Superior Direita do Project

No canto superior direito do painel inferior:

```text
[ Search ]  [ icon ] [ icon ] [ ⋮ ]
```

Elementos:

* Campo de busca.
* Botão de filtro/visualização.
* Botão de ordenação.
* Menu de opções.

---

# 10. Status Bar

## 10.1 Posição

Faixa inferior da aplicação.

```text
x: 0
y: calc(100vh - 28px)
width: 100%
height: 28px
```

## 10.2 Conteúdo

```text
⚠ 2 Warnings     ● 0 Errors      ✓ Build succeeded
```

Alinhado à esquerda.

### Estados

| Item            | Cor                |
| --------------- | ------------------ |
| Warnings        | Amarelo `#F59E0B`  |
| Errors          | Vermelho `#DC2626` |
| Build succeeded | Verde `#16A34A`    |

Estilo:

```css
background: #FFFFFF;
border-top: 1px solid #E6E8EC;
font-size: 12px;
```

---

# 11. Guia de Cores

## 11.1 Paleta Principal

```css
--color-bg-main: #F7F8FA;
--color-bg-panel: #FFFFFF;
--color-bg-muted: #F2F4F7;
--color-border: #E6E8EC;
--color-border-strong: #D5DAE1;

--color-text-primary: #1F2937;
--color-text-secondary: #687280;
--color-text-muted: #98A2B3;

--color-accent: #2563EB;
--color-accent-hover: #E8F1FF;
--color-accent-soft: #DBEAFE;

--color-success: #16A34A;
--color-warning: #F59E0B;
--color-error: #DC2626;

--color-axis-x: #EF4444;
--color-axis-y: #22C55E;
--color-axis-z: #3B82F6;
```

---

# 12. Tipografia

## 12.1 Fonte Recomendada

Usar uma fonte moderna de interface:

```css
font-family:
  Inter,
  SF Pro Text,
  Segoe UI,
  Roboto,
  Arial,
  sans-serif;
```

## 12.2 Escala Tipográfica

| Uso               | Tamanho | Peso |
| ----------------- | ------: | ---: |
| Labels pequenos   |    11px |  400 |
| Texto padrão      |    12px |  400 |
| Títulos de painel |    13px |  600 |
| Nomes de objetos  |    13px |  500 |
| Botões            |    12px |  500 |
| Header principal  |    14px |  600 |

---

# 13. Espaçamentos

```css
--spacing-xxs: 2px;
--spacing-xs: 4px;
--spacing-sm: 8px;
--spacing-md: 12px;
--spacing-lg: 16px;
--spacing-xl: 24px;
```

Regras:

* Painéis internos usam padding `8px` ou `12px`.
* Itens de árvore usam altura mínima `22px`.
* Botões principais usam altura `32px` ou `36px`.
* Separação entre componentes no Inspector: `1px border`.

---

# 14. Bordas e Sombras

```css
--radius-sm: 4px;
--radius-md: 6px;
--radius-lg: 8px;
--radius-xl: 12px;

--shadow-floating:
  0 4px 12px rgba(15, 23, 42, 0.08);

--shadow-subtle:
  0 1px 2px rgba(15, 23, 42, 0.06);
```

Uso:

* Menus flutuantes: `shadow-floating`
* Cards de asset: sombra muito sutil apenas no hover
* Botões: sem sombra por padrão
* Painéis: separados por bordas, não por sombra

---

# 15. Componentes Principais

## 15.1 Button

### Variante padrão

```css
height: 32px;
padding: 0 10px;
border: 1px solid #DDE2E8;
border-radius: 6px;
background: #FFFFFF;
color: #1F2937;
font-size: 12px;
font-weight: 500;
```

### Hover

```css
background: #F3F6FA;
```

### Ativo

```css
background: #E8F1FF;
border-color: #93C5FD;
color: #2563EB;
```

---

## 15.2 Icon Button

```css
width: 32px;
height: 32px;
display: flex;
align-items: center;
justify-content: center;
border-radius: 6px;
```

---

## 15.3 Dropdown

```css
height: 32px;
padding: 0 8px;
border: 1px solid #DDE2E8;
border-radius: 6px;
background: #FFFFFF;
font-size: 12px;
```

Deve incluir:

* Texto atual.
* Seta para baixo.
* Menu flutuante.
* Busca opcional quando houver muitos itens.

---

## 15.4 Input Numérico

```css
height: 28px;
width: 64px;
border-radius: 5px;
border: 1px solid #DDE2E8;
font-size: 12px;
text-align: center;
```

Estados:

* Focus: borda azul `#2563EB`
* Erro: borda vermelha `#DC2626`
* Disabled: fundo `#F2F4F7`

---

## 15.5 Accordion

Usado no Inspector.

```text
▾ Component Title       ⋮
content
```

Requisitos:

* Clique no header abre/fecha.
* Ícone de seta rotaciona.
* Estado salvo por objeto ou sessão.
* Suporte a menu de ações.

---

# 16. Wireframes Detalhados

## 16.1 Wireframe Geral

```text
┌──────────────────────────────────────────────────────────────────────────────────────────┐
│ LOGO PROJECT │ BRANCH │ ACTIONS                  PLAY CONTROLS         COLAB ACCOUNT ... │
├────┬────────────────────────┬────────────────────────────────────────────┬───────────────┤
│    │ HIERARCHY              │ SCENE | GAME                               │ INSPECTOR     │
│    ├────────────────────────┼────────────────────────────────────────────┼───────────────┤
│ T  │ Search                 │ Pivot Global 2D ... Gizmos Search         │ Object Header │
│ O  ├────────────────────────┤                                            ├───────────────┤
│ O  │ CityScene              │                                            │ Transform     │
│ L  │  Camera                │                                            │ Mesh Filter   │
│ B  │  Light                 │              3D CITY VIEWPORT             │ Mesh Renderer │
│ A  │  Environment           │                                            │ Lighting      │
│ R  │    Buildings           │                                            │ Probes        │
│    │      SM_Building_A_01  │                                            │ Settings      │
│    │    Props               │                                            │ Material      │
│    │    Vehicles            │                                            │ Add Component │
├────┴────────────────────────┴────────────────────────────────────────────┴───────────────┤
│ PROJECT | CONSOLE | ANIMATION                                                           │
├──────────────────────────────┬───────────────────────────────────────────────────────────┤
│ Favorites / Assets Tree      │ Breadcrumb + Asset Grid                                  │
├──────────────────────────────┴───────────────────────────────────────────────────────────┤
│ ⚠ 2 Warnings     ● 0 Errors     ✓ Build succeeded                                       │
└──────────────────────────────────────────────────────────────────────────────────────────┘
```

---

## 16.2 Wireframe do Inspector

```text
┌────────────────────────────────────────┐
│ Inspector                          🔒  │
├────────────────────────────────────────┤
│ ☑ ◇ SM_Building_A_01        Static ☐   │
│ Tag [Untagged ▼]   Layer [Default ▼]   │
├────────────────────────────────────────┤
│ ▾ Transform                            │
│ Position  X[12.34] Y[0.00] Z[-8.43]   │
│ Rotation  X[0]     Y[90]   Z[0]       │
│ Scale     X[1]     Y[1]    Z[1]       │
├────────────────────────────────────────┤
│ ▾ Mesh Filter                          │
│ Mesh      [SM_Building_A_01 ▼]         │
├────────────────────────────────────────┤
│ ▾ Mesh Renderer                        │
│ Materials                         [2]  │
│ Element 0 [M_Building_Brick ▼]         │
│ Element 1 [M_Window_Glass ▼]           │
├────────────────────────────────────────┤
│ ▾ Lighting                             │
│ Cast Shadows        [On ▼]             │
│ Receive Shadows     [☑]                │
│ Global Illumination [☐]                │
│ Light Probes        [Blend Probes ▼]   │
├────────────────────────────────────────┤
│ ▾ Probes                               │
│ Light Probes        [Blend Probes ▼]   │
│ Reflection Probes   [Blend Probes ▼]   │
│ Anchor Override     [None ×]           │
├────────────────────────────────────────┤
│ ▾ Additional Settings                  │
│ Motion Vectors      [Per Object ▼]     │
│ Dynamic Occlusion   [☑]                │
├────────────────────────────────────────┤
│ ● M_Building_Brick                     │
│ Shader [Standard ▼]            Edit... │
│                                        │
│          [ Add Component ]             │
└────────────────────────────────────────┘
```

---

## 16.3 Wireframe do Painel Project

```text
┌──────────────────────────────────────────────────────────────────────────────┐
│ Project | Console | Animation                                      Search ⋮ │
├───────────────────────┬──────────────────────────────────────────────────────┤
│ Favorites             │ Assets > Environment > Buildings                    │
│  All Materials        ├──────────────────────────────────────────────────────┤
│  All Models           │                                                      │
│  All Prefabs          │ [Building A1] [Building A2] [Building B1]            │
│  All Scripts          │                                                      │
│                       │ [Building B2] [Corner 01]  [Materials Folder]        │
│ Assets                │                                                      │
│  Environment          │                                                      │
│   Buildings           │                                                      │
│  Props                │                                                      │
│  Vehicles             │                                                      │
│  Materials            │                                                      │
│  Textures             │                                                      │
│  Scripts              │                                                      │
│  Scenes               │                                                      │
│  Settings             │                                                      │
│                       │                                                      │
│ Packages              │                                                      │
└───────────────────────┴──────────────────────────────────────────────────────┘
```

---

# 17. Comportamentos de Interação

## 17.1 Seleção de Objeto

Quando o usuário seleciona `SM_Building_A_01`:

* O item correspondente na Hierarchy recebe highlight azul claro.
* O gizmo aparece no viewport sobre o prédio.
* O Inspector carrega os componentes do objeto.
* O Project pode destacar o asset correspondente, se aplicável.

---

## 17.2 Manipulação no Viewport

Ferramentas:

| Ferramenta | Atalho | Função                |
| ---------- | ------ | --------------------- |
| Selecionar | Q      | Seleção/navegação     |
| Mover      | W      | Move objeto           |
| Rotacionar | E      | Rotaciona objeto      |
| Escalar    | R      | Escala objeto         |
| Transform  | T      | Manipulador combinado |

Regras:

* Arrastar eixo X move apenas no eixo X.
* Arrastar eixo Y move apenas no eixo Y.
* Arrastar eixo Z move apenas no eixo Z.
* Arrastar plano entre eixos move em dois eixos.
* Valores devem atualizar no Inspector em tempo real.

---

## 17.3 Drag-and-drop de Assets

Deve ser possível:

* Arrastar prefab do Project para o Viewport.
* Arrastar material para slot no Inspector.
* Arrastar mesh para `Mesh Filter`.
* Arrastar objeto na Hierarchy para mudar parent.

---

## 17.4 Menus Contextuais

### Hierarchy

Opções:

```text
Create Empty
Create Camera
Create Light
Rename
Duplicate
Delete
Focus
Copy Path
```

### Viewport

Opções:

```text
Frame Selected
Create Object
Paste
Align View to Selected
Toggle Gizmos
```

### Project

Opções:

```text
Create Folder
Import Asset
Rename
Duplicate
Delete
Show in Explorer
Copy Path
Reimport
```

### Inspector

Opções:

```text
Reset Component
Copy Component
Paste Component Values
Remove Component
Move Up
Move Down
```

---

# 18. Estados Visuais

## 18.1 Hover

* Botões: fundo cinza claro.
* Itens de árvore: fundo `#F3F6FA`.
* Assets: borda azul suave.
* Inputs: borda levemente mais escura.

## 18.2 Focus

* Inputs e dropdowns recebem contorno azul.
* Espessura recomendada: `2px`.
* Cor: `#2563EB`.

## 18.3 Disabled

```css
opacity: 0.55;
cursor: not-allowed;
background: #F2F4F7;
```

## 18.4 Selected

```css
background: #E8F1FF;
border-color: #93C5FD;
color: #1D4ED8;
```

---

# 19. Responsividade

## 19.1 Desktop Grande

Para telas acima de `1600px`:

* Inspector: `380px`
* Hierarchy: `340px`
* Project panel: `260px` altura
* Viewport ocupa o restante

## 19.2 Desktop Médio

Para telas entre `1280px` e `1599px`:

* Inspector: `320px`
* Hierarchy: `280px`
* Toolbar: `48px`
* Painel inferior: `220px`

## 19.3 Tela Pequena

Para telas abaixo de `1280px`:

* Inspector pode ser recolhível.
* Hierarchy pode virar aba lateral.
* Project panel pode reduzir altura.
* Header deve esconder textos secundários e manter apenas ícones.

---

# 20. Sistema de Layout Redimensionável

A interface deve permitir redimensionar:

* Largura da Hierarchy.
* Largura do Inspector.
* Altura do painel inferior.
* Proporção entre Project/Console/Animation.

Divisores:

```css
width: 4px;
cursor: col-resize;
background: transparent;
```

Hover do divisor:

```css
background: #BFDBFE;
```

---

# 21. Atalhos de Teclado

| Atalho               | Ação                     |
| -------------------- | ------------------------ |
| Q                    | Selecionar               |
| W                    | Mover                    |
| E                    | Rotacionar               |
| R                    | Escalar                  |
| T                    | Transform universal      |
| F                    | Focar objeto selecionado |
| Delete               | Remover objeto           |
| Ctrl/Cmd + D         | Duplicar                 |
| Ctrl/Cmd + Z         | Undo                     |
| Ctrl/Cmd + Shift + Z | Redo                     |
| Ctrl/Cmd + S         | Salvar                   |
| Space                | Play/Pause               |
| Ctrl/Cmd + P         | Buscar projeto           |
| Ctrl/Cmd + F         | Buscar no painel ativo   |

---

# 22. Dados Mockados para Implementação Inicial

## 22.1 Scene Hierarchy

```json
{
  "name": "CityScene",
  "type": "scene",
  "children": [
    { "name": "Main Camera", "type": "camera" },
    { "name": "Directional Light", "type": "light" },
    { "name": "Sky and Fog Volume", "type": "volume" },
    {
      "name": "Environment",
      "type": "folder",
      "children": [
        {
          "name": "Buildings",
          "type": "folder",
          "children": [
            { "name": "SM_Building_A_01", "type": "mesh" },
            { "name": "SM_Building_A_02", "type": "mesh" },
            { "name": "SM_Building_B_01", "type": "mesh" },
            { "name": "SM_Building_Corner_01", "type": "mesh" }
          ]
        },
        {
          "name": "Props",
          "type": "folder",
          "children": [
            { "name": "SM_Street_Lamp_01", "type": "mesh" },
            { "name": "SM_Bench_01", "type": "mesh" },
            { "name": "SM_TrashCan_01", "type": "mesh" },
            { "name": "SM_Planter_01", "type": "mesh" }
          ]
        },
        {
          "name": "Vehicles",
          "type": "folder",
          "children": [
            { "name": "SM_Car_01", "type": "mesh" },
            { "name": "SM_Car_02", "type": "mesh" },
            { "name": "SM_Truck_01", "type": "mesh" }
          ]
        }
      ]
    },
    {
      "name": "Gameplay",
      "type": "folder",
      "children": [
        { "name": "PlayerStart", "type": "spawn" },
        { "name": "Waypoints", "type": "path" }
      ]
    },
    { "name": "PostProcessing Volume", "type": "volume" },
    { "name": "Audio Listener", "type": "audio" }
  ]
}
```

---

## 22.2 Objeto Selecionado

```json
{
  "name": "SM_Building_A_01",
  "enabled": true,
  "static": false,
  "tag": "Untagged",
  "layer": "Default",
  "transform": {
    "position": { "x": 12.34, "y": 0.0, "z": -8.43 },
    "rotation": { "x": 0, "y": 90, "z": 0 },
    "scale": { "x": 1, "y": 1, "z": 1 }
  },
  "meshFilter": {
    "mesh": "SM_Building_A_01"
  },
  "meshRenderer": {
    "materials": [
      "M_Building_Brick",
      "M_Window_Glass"
    ]
  },
  "lighting": {
    "castShadows": "On",
    "receiveShadows": true,
    "contributeGlobalIllumination": false,
    "lightProbes": "Blend Probes"
  },
  "probes": {
    "lightProbes": "Blend Probes",
    "reflectionProbes": "Blend Probes",
    "anchorOverride": null
  },
  "additionalSettings": {
    "motionVectors": "Per Object Motion",
    "dynamicOcclusion": true
  }
}
```

---

## 22.3 Assets

```json
[
  {
    "name": "SM_Building_A_01",
    "type": "model",
    "path": "Assets/Environment/Buildings/SM_Building_A_01"
  },
  {
    "name": "SM_Building_A_02",
    "type": "model",
    "path": "Assets/Environment/Buildings/SM_Building_A_02"
  },
  {
    "name": "SM_Building_B_01",
    "type": "model",
    "path": "Assets/Environment/Buildings/SM_Building_B_01"
  },
  {
    "name": "SM_Building_B_02",
    "type": "model",
    "path": "Assets/Environment/Buildings/SM_Building_B_02"
  },
  {
    "name": "SM_Building_Corner_01",
    "type": "model",
    "path": "Assets/Environment/Buildings/SM_Building_Corner_01"
  },
  {
    "name": "Materials",
    "type": "folder",
    "path": "Assets/Environment/Buildings/Materials"
  }
]
```

---

# 23. Requisitos de Implementação Frontend

## 23.1 Stack Recomendada

A interface pode ser implementada com:

* React
* TypeScript
* Tailwind CSS
* Zustand ou Redux para estado global
* React Three Fiber para viewport 3D
* Drei para controles 3D
* Radix UI para dropdowns, menus e accordions
* Lucide Icons para ícones
* Framer Motion para transições sutis

---

## 23.2 Estrutura de Componentes

```text
src/
  components/
    layout/
      AppShell.tsx
      Header.tsx
      SidebarToolbar.tsx
      ResizablePanel.tsx
      StatusBar.tsx

    hierarchy/
      HierarchyPanel.tsx
      HierarchyTree.tsx
      HierarchyItem.tsx
      HierarchySearch.tsx

    viewport/
      SceneViewport.tsx
      ViewportTabs.tsx
      ViewportToolbar.tsx
      TransformGizmo.tsx
      OrientationGizmo.tsx
      ViewportToolPalette.tsx

    inspector/
      InspectorPanel.tsx
      ObjectHeader.tsx
      TransformComponent.tsx
      MeshFilterComponent.tsx
      MeshRendererComponent.tsx
      LightingComponent.tsx
      ProbesComponent.tsx
      AdditionalSettingsComponent.tsx
      MaterialPreview.tsx

    project/
      ProjectPanel.tsx
      ProjectTabs.tsx
      ProjectSidebar.tsx
      AssetGrid.tsx
      AssetCard.tsx
      Breadcrumbs.tsx

    common/
      Button.tsx
      IconButton.tsx
      Dropdown.tsx
      NumericInput.tsx
      AccordionSection.tsx
      SearchInput.tsx
      Checkbox.tsx
      ContextMenu.tsx
```

---

# 24. Requisitos de UX

## 24.1 Prioridades

A interface deve priorizar:

1. Clareza visual.
2. Resposta rápida.
3. Densidade de informação.
4. Consistência de seleção.
5. Baixo atrito para manipulação 3D.
6. Familiaridade para usuários de engines.

---

## 24.2 Feedback Imediato

Todas as ações devem gerar feedback visual:

* Seleção atual destacada.
* Inputs alterados atualizam o viewport.
* Drag-and-drop mostra target válido.
* Erros aparecem no Console e Status Bar.
* Build status aparece imediatamente.

---

## 24.3 Persistência de Layout

Salvar por usuário:

* Larguras dos painéis.
* Altura do painel inferior.
* Tabs abertas.
* Painéis recolhidos.
* Ferramenta ativa.
* Modo Scene/Game.
* Estado dos accordions do Inspector.

---

# 25. Requisitos de Acessibilidade

* Todos os botões devem ter `aria-label`.
* Inputs devem ter labels associadas.
* Navegação por teclado nos painéis.
* Focus ring visível.
* Contraste mínimo WCAG AA.
* Tooltips para ícones.
* Não depender apenas de cor para estados importantes.

---

# 26. Guia Visual da Cena 3D

A cena de demonstração deve transmitir um ambiente urbano realista.

## 26.1 Câmera

* Perspectiva em ângulo elevado.
* Aproximadamente 35º a 45º acima do solo.
* Visão voltada para esquina urbana.
* FOV aproximado: `45º a 60º`.

## 26.2 Iluminação

* Luz solar direcional.
* Sombras suaves.
* Céu azul claro.
* Nuvens brancas.
* Ambiente levemente frio.

## 26.3 Assets Visíveis

* Prédio selecionado na esquina.
* Prédios adjacentes.
* Fachadas de tijolos.
* Janelas com material reflexivo.
* Toldos verdes.
* Ruas com marcações.
* Calçadas.
* Árvores.
* Carros.
* Caminhão branco.
* Táxi amarelo.
* Poste de iluminação.
* Hidrante.

---

# 27. Checklist de Desenvolvimento

## 27.1 Layout

* [ ] Header global implementado.
* [ ] Toolbar lateral implementada.
* [ ] Hierarchy redimensionável.
* [ ] Viewport central.
* [ ] Inspector redimensionável.
* [ ] Painel inferior redimensionável.
* [ ] Status bar fixa.

## 27.2 Funcionalidades

* [ ] Seleção na Hierarchy.
* [ ] Seleção no Viewport.
* [ ] Sincronização com Inspector.
* [ ] Inputs de Transform funcionais.
* [ ] Project asset grid.
* [ ] Breadcrumbs funcionais.
* [ ] Tabs Scene/Game.
* [ ] Tabs Project/Console/Animation.
* [ ] Menus contextuais.
* [ ] Drag-and-drop.
* [ ] Atalhos de teclado.

## 27.3 Visual

* [ ] Cores conforme guia.
* [ ] Tipografia aplicada.
* [ ] Ícones consistentes.
* [ ] Estados hover/selected/focus.
* [ ] Gizmos coloridos corretamente.
* [ ] Thumbnails de assets.
* [ ] Material preview.

---

# 28. Critérios de Aceite

A implementação será considerada fiel quando:

1. A composição geral reproduzir a estrutura da imagem de referência.
2. O header ocupar a faixa superior com controles centralizados.
3. A Hierarchy estiver à esquerda com tree view funcional.
4. O viewport 3D ocupar a área central dominante.
5. O Inspector exibir exatamente os componentes do objeto selecionado.
6. O painel Project inferior mostrar árvore lateral, breadcrumbs e grade de assets.
7. A Status Bar exibir warnings, errors e build status.
8. A seleção de `SM_Building_A_01` refletir em todos os painéis.
9. Os estilos visuais seguirem a paleta clara, técnica e moderna.
10. O layout permitir redimensionamento e manter boa legibilidade.

---

# 29. Observações Finais

A interface deve parecer uma ferramenta profissional de criação e edição de cenas 3D, não apenas um mockup visual.
O foco principal é entregar uma experiência de engine/editor, com comportamento previsível, alta precisão visual e estrutura modular para evolução futura.

A primeira versão pode usar dados mockados, mas a arquitetura deve permitir posterior integração com:

* Motor 3D real.
* Sistema de assets.
* Build pipeline.
* Colaboração multiplayer.
* Controle de versão.
* Editor de materiais.
* Sistema de layers.
* Console de logs.
* Sistema de plugins.

```
```
