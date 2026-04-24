# Borealis Desktop Linker Profile v1

## Objetivo

Definir um profile simples para ligar um backend desktop (Raylib/OpenGL) sem quebrar o onboarding.

## Como funciona no runtime atual

1. O app pede `backend_id=1` (perfil desktop).
2. O runtime tenta carregar Raylib dinamicamente (`raylib.dll`/`libraylib.so`/`libraylib.dylib`) e registra um adapter builtin.
3. Se nao houver adapter registrado, o runtime faz fallback seguro para `stub`.

Isso garante que o projeto continua rodando mesmo sem o backend real linkado.

## Layout nativo do modulo

Raylib deve ficar fora de `src/`, em uma pasta nativa do modulo:

```text
packages/borealis/native/raylib/<plataforma>/
```

Exemplos:

- Windows x64: `packages/borealis/native/raylib/windows-x64/raylib.dll`
- Windows x64 (layout oficial do release): `packages/borealis/native/raylib/windows-x64/lib/raylib.dll`
- Linux x64: `packages/borealis/native/raylib/linux-x64/libraylib.so`
- macOS arm64: `packages/borealis/native/raylib/macos-arm64/libraylib.dylib`

O runtime tambem aceita `BOREALIS_RAYLIB_PATH` ou `ZENITH_RAYLIB_PATH` quando
o binario estiver fora do modulo.

## Hook de linker (contrato C)

Contrato novo no runtime C (`runtime/c/zenith_rt.h`):

- `zt_borealis_desktop_api` (tabela de funcoes do backend desktop).
- `zt_borealis_set_desktop_api(...)` (registro do adapter).
- `zt_borealis_get_desktop_api(...)` (leitura do adapter atual).

## Implementacao minima do adapter

O adapter desktop precisa:

1. Implementar as funcoes do contrato (`open_window`, `begin_frame`, `draw_*`, `is_key_*`, etc).
2. Criar uma tabela `zt_borealis_desktop_api`.
3. Registrar essa tabela com `zt_borealis_set_desktop_api(&api)`.

Exemplo minimo (C):

```c
#include "runtime/c/zenith_rt.h"

static zt_outcome_i64_core_error my_open_window(
    const zt_text *title,
    zt_int width,
    zt_int height,
    zt_int target_fps,
    zt_int backend_id
) {
    /* implementar com Raylib */
}

static zt_borealis_desktop_api g_api = {
    my_open_window,
    /* close_window */
    /* window_should_close */
    /* begin_frame */
    /* end_frame */
    /* draw_rect */
    /* draw_line */
    /* draw_rect_outline */
    /* draw_circle */
    /* draw_circle_outline */
    /* draw_text */
    /* is_key_down */
    /* is_key_pressed */
    /* is_key_released */
};

void borealis_desktop_register_adapter(void) {
    zt_borealis_set_desktop_api(&g_api);
}
```

## Linker flags (referencia)

Com o adapter Raylib builtin por carga dinamica, o fluxo padrao nao exige linkar Raylib no build do app.

Os flags abaixo sao referencia para cenarios de adapter externo/manual:

Windows (MinGW), exemplo tipico:

- `-lraylib -lopengl32 -lgdi32 -lwinmm`

Linux, exemplo tipico:

- `-lraylib -lGL -lm -lpthread -ldl -lrt -lX11`

Observacao: os flags exatos dependem da instalacao do Raylib no ambiente.

## Estado atual

- Hook de adapter desktop: pronto.
- Adapter Raylib inicial (janela/input/draw): pronto via carregamento dinamico.
- Modulo `borealis.raylib` com binding Zenith inicial: pronto.
- Busca vendorizada em `packages/borealis/native/raylib`: pronta.
- API de diagnostico `raylib_available`/`raylib_loaded_path`: pronta.
- Fallback seguro para stub: pronto.
- Teste E2E no caminho desktop->fallback: pronto.
- Smoke test do binding `borealis.raylib` (shapes/text/input + textura/som em caminho stub-safe): pronto.
- Probe de assets reais (`.png` + `.wav`) com Raylib nativa: pronto no workspace local.
- Layout vendorizado oficial `windows-x64/lib/raylib.dll`: validado no workspace local.
- Validacao com Raylib real depende do ambiente ter a biblioteca instalada.
