import type { StudioHome, StudioSnapshot, Transform3D } from "./types";

const baseTransform: Transform3D = {
  x: 0,
  y: 0,
  z: 0,
  rotationX: 0,
  rotationY: 0,
  rotationZ: 0,
  scaleX: 1,
  scaleY: 1,
  scaleZ: 1,
};

export function createMockSnapshot(): StudioSnapshot {
  return {
    projectName: "Borealis",
    projectPath: "packages/borealis/zenith.ztproj",
    projectRoot: "packages/borealis",
    scene: {
      name: "sample_3d",
      path: "packages/borealis/scenes/sample_3d.scene.json",
      documentId: "scene:sample_3d",
      environment: {
        skybox: { mode: "solid", color: "#1c1f26" },
        ambient: { color: "#ffffff", intensity: 0.4 },
        fog: { enabled: false, color: "#9ca3af", density: 0.05 },
        weather: { preset: "clear" },
      },
      render: {
        quality: { profile: "medium" },
        postfx: { fxaa: false, bloom: 0, vignette: 0 },
        camera: {},
      },
      audio: {
        listener: {},
        mix: { master: 1, music: 0.8, sfx: 1 },
      },
      entities: [
        {
          id: "camera-3d",
          name: "Camera 3D",
          layer: "world3d",
          tags: ["camera", "3d"],
          components: [{ kind: "camera3d", profile: "perspective", properties: { projection: "perspective" } }],
          transform: {
            ...baseTransform,
            x: 120,
            y: -120,
            z: 120,
            rotationX: -35,
            rotationY: 35,
          },
        },
        {
          id: "player",
          name: "Player",
          layer: "actors",
          tags: ["player", "controllable"],
          components: [
            { kind: "model3d", asset: "assets/triangle.obj", properties: { asset: "assets/triangle.obj" } },
            { kind: "script", script: "src/app/player_controller.zt", properties: { script: "src/app/player_controller.zt" } },
          ],
          transform: {
            ...baseTransform,
            x: 28,
            y: 18,
            z: 24,
          },
        },
        {
          id: "cube-01",
          name: "Cube 01",
          layer: "world3d",
          parent: "player",
          tags: ["3d", "prop"],
          components: [
            { kind: "cube3d", properties: { size: 48 } },
            { kind: "script", script: "src/app/spinning_cube.zt", properties: { script: "src/app/spinning_cube.zt" } },
          ],
          transform: {
            ...baseTransform,
            x: 164,
            y: 82,
            z: 18,
            scaleX: 1.2,
            scaleY: 1.2,
            scaleZ: 1.2,
          },
        },
      ],
    },
    assets: [
      {
        id: "scene-sample-3d",
        name: "sample_3d.scene.json",
        path: "scenes/sample_3d.scene.json",
        kind: "scene",
      },
      {
        id: "asset-triangle",
        name: "triangle.obj",
        path: "assets/triangle.obj",
        kind: "model",
      },
      {
        id: "script-player",
        name: "player_controller.zt",
        path: "src/app/player_controller.zt",
        kind: "script",
      },
      {
        id: "script-cube",
        name: "spinning_cube.zt",
        path: "src/app/spinning_cube.zt",
        kind: "script",
      },
    ],
    scripts: [
      {
        name: "player_controller.zt",
        path: "src/app/player_controller.zt",
        content: [
          "namespace app.player_controller",
          "",
          "import borealis.game as game",
          "import borealis.game.input as input",
          "",
          "public func update(entity_id: text, dt: float) -> result<void, core.Error>",
          "    const speed: float = 140.0",
          "    var move_x: float = 0.0",
          "",
          "    if input.key_down(\"A\")",
          "        move_x = move_x - speed * dt",
          "    end",
          "    if input.key_down(\"D\")",
          "        move_x = move_x + speed * dt",
          "    end",
          "",
          "    game.entity_translate(entity_id, move_x, 0.0)?",
          "    return success()",
          "end",
        ].join("\n"),
      },
      {
        name: "spinning_cube.zt",
        path: "src/app/spinning_cube.zt",
        content: [
          "namespace app.spinning_cube",
          "",
          "import borealis.game as game",
          "",
          "public func update(entity_id: text, dt: float) -> result<void, core.Error>",
          "    const degrees: float = 45.0 * dt",
          "    game.entity_rotate_y(entity_id, degrees)?",
          "    return success()",
          "end",
        ].join("\n"),
      },
    ],
    console: [
      {
        id: "line-1",
        level: "info",
        source: "studio",
        message: "Borealis Studio initialized with browser fallback data.",
      },
      {
        id: "line-2",
        level: "info",
        source: "preview",
        message: "Scene sample_3d loaded. 3 entities available.",
      },
      {
        id: "line-3",
        level: "warn",
        source: "compiler",
        message: "Runtime preview is not attached in browser mode.",
      },
    ],
  };
}

export function createMockHome(): StudioHome {
  return {
    workspaceRoot: "browser://borealis-studio",
    appRoot: "browser://borealis-studio",
    repoRoot: undefined,
    sdkRoot: undefined,
    runtimeMode: "missing",
    runtimeStatus: "Runtime desktop indisponivel no modo browser.",
    editorManifest: {
      version: 1,
      source: "fallback:browser-mock",
      components: {},
      sceneSettings: {},
      assetKinds: {},
      actions: {},
      templates: {},
    },
    defaultProjectPath: "packages/borealis/zenith.ztproj",
    defaultProjectsDir: "Borealis Projects",
    templates: [
      {
        id: "empty3d",
        name: "Empty 3D",
        summary: "Cena 3D limpa com camera, cubo e espaco para os primeiros sistemas.",
        defaultName: "Projeto Borealis 3D",
        tags: ["3D", "starter"],
      },
      {
        id: "topdown2d",
        name: "Top-down 2D",
        summary: "Cena 2D pequena para movimento, sprites e testes iniciais de gameplay.",
        defaultName: "Projeto Borealis 2D",
        tags: ["2D", "gameplay"],
      },
      {
        id: "scripted3d",
        name: "Scripted 3D",
        summary: "Amostra 3D com script anexado para iterar no preview.",
        defaultName: "Projeto Borealis Scripted",
        tags: ["3D", "scripts"],
      },
    ],
    docs: [
      {
        title: "Studio README",
        path: "tools/borealis-studio/README.md",
        summary: "Comandos, escopo atual e fluxo de preview.",
      },
      {
        title: "Stack Decision",
        path: "packages/borealis/decisions/015-borealis-studio-tauri-react-stack.md",
        summary: "Direcao ativa com Tauri + React.",
      },
      {
        title: "Preview IPC",
        path: "packages/borealis/decisions/014-borealis-editor-preview-ipc.md",
        summary: "Protocolo JSONL usado pelo Play mode.",
      },
    ],
  };
}
