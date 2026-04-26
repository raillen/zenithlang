#!/usr/bin/env python3
"""
Assemble the local Borealis Studio SDK.

This creates the runtime/sdk folder used by the standalone Studio. It copies
versioned runtime artifacts, not the full Zenith repository.
"""

from __future__ import annotations

import argparse
import datetime as dt
import json
import os
import shutil
import subprocess
import sys
from pathlib import Path


SCRIPT_DIR = Path(__file__).resolve().parent
STUDIO_ROOT = SCRIPT_DIR.parent
TOOLS_ROOT = STUDIO_ROOT.parent
DEFAULT_REPO_ROOT = TOOLS_ROOT.parent
DEFAULT_SDK_DIR = STUDIO_ROOT / "runtime" / "sdk"

MANAGED_NAMES = [
    "bin",
    "editor",
    "packages",
    "preview",
    "runtime",
    "stdlib",
    "templates",
    "borealis.editor.json",
    "sdk-manifest.json",
]

PREVIEW_PROJECT = """[project]
name = "borealis_preview"
version = "0.1.0"
kind = "app"

[source]
root = "src"

[app]
entry = "app.main"

[build]
target = "native"
output = "build"
profile = "debug"
"""

PREVIEW_MAIN = r'''namespace app.main

import std.io as io
import std.text as text

extern c
    func zt_host_read_file(file_path: text) -> result<text, core.Error>
    func zt_borealis_open_window(title: text, width: int, height: int, target_fps: int, backend_id: int) -> result<int, core.Error>
    func zt_borealis_close_window(window_id: int) -> result<void, core.Error>
    func zt_borealis_window_should_close(window_id: int) -> bool
    func zt_borealis_begin_frame(window_id: int, clear_r: int, clear_g: int, clear_b: int, clear_a: int) -> result<void, core.Error>
    func zt_borealis_end_frame(window_id: int) -> result<void, core.Error>
    func zt_borealis_draw_rect(window_id: int, x: float, y: float, width: float, height: float, color_r: int, color_g: int, color_b: int, color_a: int) -> result<void, core.Error>
    func zt_borealis_draw_rect_outline(window_id: int, x: float, y: float, width: float, height: float, thickness: float, color_r: int, color_g: int, color_b: int, color_a: int) -> result<void, core.Error>
    func zt_borealis_draw_circle(window_id: int, x: float, y: float, radius: float, color_r: int, color_g: int, color_b: int, color_a: int) -> result<void, core.Error>
    func zt_borealis_draw_line(window_id: int, x1: float, y1: float, x2: float, y2: float, color_r: int, color_g: int, color_b: int, color_a: int) -> result<void, core.Error>
    func zt_borealis_draw_text(window_id: int, value: text, x: int, y: int, size: int, color_r: int, color_g: int, color_b: int, color_a: int) -> result<void, core.Error>
end

var loaded_scene_path: text = ""
var loaded_entity_count: int = 0
var loaded_camera_count: int = 0
var loaded_light_count: int = 0
var loaded_audio_count: int = 0
var loaded_has_postfx: bool = false

func emit(channel: text, kind: text, payload: text) -> result<void, core.Error>
    io.write("{\"protocol\":1,\"seq\":1,\"channel\":\"")?
    io.write(channel)?
    io.write("\",\"kind\":\"")?
    io.write(kind)?
    io.write("\",\"payload\":")?
    io.write(payload)?
    io.write("}\n")?
    return success()
end

func emit_status(status: text, message: text) -> result<void, core.Error>
    io.write("{\"protocol\":1,\"seq\":1,\"channel\":\"event\",\"kind\":\"status\",\"payload\":{\"status\":\"")?
    io.write(status)?
    io.write("\",\"message\":\"")?
    io.write(message)?
    io.write("\"}}\n")?
    return success()
end

func extract_path(line: text) -> text
    const marker: text = "\"path\":\""
    const marker_index: int = text.index_of(line, marker)
    if marker_index < 0
        return ""
    end

    const value_start: int = marker_index + len(marker)
    if value_start >= len(line)
        return ""
    end

    const rest: text = line[value_start..(len(line) - 1)]
    const value_end: int = text.index_of(rest, "\"")
    if value_end <= 0
        return ""
    end

    return rest[0..(value_end - 1)]
end

func count_occurrences(value: text, needle: text) -> int
    var remaining: text = value
    var count: int = 0

    if len(needle) == 0
        return 0
    end

    while len(remaining) >= len(needle)
        const found: int = text.index_of(remaining, needle)
        if found < 0
            return count
        end

        count = count + 1
        const next_index: int = found + len(needle)
        if next_index >= len(remaining)
            return count
        end
        remaining = remaining[next_index..(len(remaining) - 1)]
    end

    return count
end

func emit_scene_loaded(scene_path: text) -> result<void, core.Error>
    match zt_host_read_file(scene_path)
        case success(raw) ->
            const entity_count: int = count_occurrences(raw, "\"stable_id\"")
            const camera_count: int = count_occurrences(raw, "camera2d") + count_occurrences(raw, "camera3d")
            const light_count: int = count_occurrences(raw, "directional_light") + count_occurrences(raw, "point_light") + count_occurrences(raw, "spot_light")
            const audio_count: int = count_occurrences(raw, "\"kind\":\"audio\"") + count_occurrences(raw, "\"kind\": \"audio\"") + count_occurrences(raw, "audio3d")
            const has_postfx: bool = text.contains(raw, "\"postfx\"")
            loaded_scene_path = scene_path
            loaded_entity_count = entity_count
            loaded_camera_count = camera_count
            loaded_light_count = light_count
            loaded_audio_count = audio_count
            loaded_has_postfx = has_postfx
            const payload: text = fmt "{{\"message\":\"Scene v2 loaded\",\"loaded\":true,\"path\":\"{scene_path}\",\"entity_count\":{entity_count},\"camera_count\":{camera_count},\"light_count\":{light_count},\"audio_count\":{audio_count},\"postfx\":{has_postfx}}}"
            emit("event", "diagnostic", payload)?
            emit_status("ready", "Scene ready")?
            return success()
        case error(err) ->
            const payload: text = fmt "{{\"message\":\"Scene load failed\",\"path\":\"{scene_path}\"}}"
            emit("error", "diagnostic", payload)?
            emit_status("error", "Scene failed")?
            return success()
    end
end

func draw_preview_grid(window_id: int) -> result<void, core.Error>
    var line: int = 0
    var x: float = 72.0
    while line < 12
        zt_borealis_draw_line(window_id, x, 108.0, x, 492.0, 45, 55, 70, 255)?
        x = x + 56.0
        line = line + 1
    end

    line = 0
    var y: float = 108.0
    while line < 7
        zt_borealis_draw_line(window_id, 72.0, y, 688.0, y, 45, 55, 70, 255)?
        y = y + 56.0
        line = line + 1
    end

    return success()
end

func draw_preview_entities(window_id: int) -> result<void, core.Error>
    var index: int = 0
    var column: int = 0
    var x: float = 150.0
    var y: float = 185.0
    const count: int = loaded_entity_count

    while index < count and index < 12
        zt_borealis_draw_rect(window_id, x, y, 52.0, 52.0, 245, 165, 36, 255)?
        zt_borealis_draw_rect_outline(window_id, x - 2.0, y - 2.0, 56.0, 56.0, 2.0, 255, 235, 190, 255)?
        index = index + 1
        column = column + 1
        if column >= 4
            column = 0
            x = 150.0
            y = y + 78.0
        else
            x = x + 118.0
        end
    end

    if loaded_camera_count > 0
        zt_borealis_draw_circle(window_id, 112.0, 138.0, 16.0, 94, 234, 160, 255)?
        zt_borealis_draw_text(window_id, "Camera", 136, 127, 18, 210, 245, 225, 255)?
    end

    if loaded_light_count > 0
        zt_borealis_draw_circle(window_id, 615.0, 138.0, 18.0, 255, 221, 87, 255)?
        zt_borealis_draw_line(window_id, 615.0, 166.0, 615.0, 210.0, 255, 221, 87, 255)?
        zt_borealis_draw_text(window_id, "Light", 640, 127, 18, 255, 241, 180, 255)?
    end

    return success()
end

func run_visual_preview() -> result<void, core.Error>
    match zt_borealis_open_window("Borealis Play Mode", 960, 540, 60, 1)
        case success(window_id) ->
            emit_status("playing", "Visual runtime started")?
            const summary: text = fmt "{{\"message\":\"Visual runtime opened\",\"entity_count\":{loaded_entity_count},\"camera_count\":{loaded_camera_count},\"light_count\":{loaded_light_count},\"audio_count\":{loaded_audio_count}}}"
            emit("event", "diagnostic", summary)?

            var frame_count: int = 0
            while frame_count < 180
                if zt_borealis_window_should_close(window_id)
                    break
                end

                zt_borealis_begin_frame(window_id, 18, 24, 32, 255)?
                draw_preview_grid(window_id)?
                draw_preview_entities(window_id)?
                zt_borealis_draw_text(window_id, "Borealis Play Mode", 24, 22, 24, 245, 248, 250, 255)?
                zt_borealis_draw_text(window_id, loaded_scene_path, 24, 54, 16, 170, 185, 205, 255)?
                zt_borealis_draw_text(window_id, fmt "Entities: {loaded_entity_count}", 24, 488, 18, 245, 248, 250, 255)?
                zt_borealis_draw_text(window_id, fmt "Cameras: {loaded_camera_count}", 172, 488, 18, 210, 245, 225, 255)?
                zt_borealis_draw_text(window_id, fmt "Lights: {loaded_light_count}", 320, 488, 18, 255, 241, 180, 255)?
                if loaded_has_postfx
                    zt_borealis_draw_text(window_id, "PostFX", 452, 488, 18, 190, 210, 255, 255)?
                end
                zt_borealis_end_frame(window_id)?
                frame_count = frame_count + 1
            end

            zt_borealis_close_window(window_id)?
            emit_status("stopped", "Visual runtime stopped")?
            return success()
        case error(err) ->
            emit("error", "diagnostic", "{\"message\":\"Visual runtime failed to open\"}")?
            emit_status("error", "Visual runtime failed")?
            return success()
    end
end

func handle_command(line: text) -> result<bool, core.Error>
    if text.contains(line, "\"kind\":\"hello\"")
        emit("response", "hello", "{\"status\":\"ready\",\"message\":\"Borealis preview online\"}")?
        return success(true)
    end

    if text.contains(line, "\"kind\":\"open_project\"")
        emit_status("loading", "Project received")?
        return success(true)
    end

    if text.contains(line, "\"kind\":\"open_scene\"")
        const scene_path: text = extract_path(line)
        emit_scene_loaded(scene_path)?
        return success(true)
    end

    if text.contains(line, "\"kind\":\"enter_play_mode\"")
        run_visual_preview()?
        return success(true)
    end

    if text.contains(line, "\"kind\":\"pause_play_mode\"")
        emit_status("paused", "Play mode paused")?
        return success(true)
    end

    if text.contains(line, "\"kind\":\"stop_play_mode\"")
        emit_status("stopped", "Play mode stopped")?
        return success(false)
    end

    emit("event", "diagnostic", "{\"message\":\"Command accepted\"}")?
    return success(true)
end

func main() -> result<void, core.Error>
    emit_status("idle", "Borealis preview runner started")?

    var running: bool = true
    while running
        const maybe_line: optional<text> = io.read_line()?
        match maybe_line
            case value line ->
                running = handle_command(line)?
            case none ->
                running = false
        end
    end

    return success()
end
'''

TEMPLATE_3D = {
    "id": "starter-3d",
    "name": "Starter 3D",
    "summary": "Projeto 3D minimo com camera, luz e cubo.",
    "defaultName": "Projeto Borealis 3D",
    "tags": ["3D", "starter", "SDK"],
    "scene": "templates/starter-3d.scene.json",
    "mainScript": "templates/starter-main.zt",
    "assets": [
        {"source": "templates/triangle.obj", "target": "assets/triangle.obj"},
    ],
}

TEMPLATE_2D = {
    "id": "topdown2d",
    "name": "Top-down 2D",
    "summary": "Cena 2D pequena para movimento, sprites e testes iniciais de gameplay.",
    "defaultName": "Projeto Borealis 2D",
    "tags": ["2D", "gameplay", "SDK"],
}

TEMPLATE_SCRIPTED = {
    "id": "scripted3d",
    "name": "Scripted 3D",
    "summary": "Amostra 3D com script anexado para iterar no preview.",
    "defaultName": "Projeto Borealis Scripted",
    "tags": ["3D", "scripts", "SDK"],
}

STARTER_3D_SCENE = {
    "version": 2,
    "name": "Starter 3D",
    "document_id": "starter-3d",
    "environment": {
        "skybox": {"type": "color", "color": "#121820"},
        "ambient": {"color": "#ffffff", "intensity": 0.35},
        "fog": {"enabled": False, "color": "#121820", "density": 0.01},
        "weather": {"profile": "clear", "intensity": 0},
    },
    "render": {
        "postfx": {
            "enabled": False,
            "bloom": 0,
            "vignette": 0,
            "color_grading": "neutral",
        }
    },
    "audio": {"master": 1, "music": 0.8, "sfx": 0.9, "spatial": True},
    "entities": [
        {
            "stable_id": "camera-main",
            "name": "Main Camera",
            "layer": "default",
            "parent": None,
            "tags": ["camera"],
            "transform": {
                "x": 0,
                "y": 4,
                "z": 8,
                "rotation_x": -20,
                "rotation_y": 0,
                "rotation_z": 0,
                "scale_x": 1,
                "scale_y": 1,
                "scale_z": 1,
            },
            "components": [
                {
                    "kind": "camera3d",
                    "asset": None,
                    "script": None,
                    "profile": None,
                    "properties": {
                        "projection": "perspective",
                        "fov": 60,
                        "near": 0.1,
                        "far": 1000,
                    },
                }
            ],
        },
        {
            "stable_id": "sun",
            "name": "Sun",
            "layer": "default",
            "parent": None,
            "tags": ["light"],
            "transform": {
                "x": -3,
                "y": 6,
                "z": 4,
                "rotation_x": -45,
                "rotation_y": -30,
                "rotation_z": 0,
                "scale_x": 1,
                "scale_y": 1,
                "scale_z": 1,
            },
            "components": [
                {
                    "kind": "directional_light",
                    "asset": None,
                    "script": None,
                    "profile": None,
                    "properties": {
                        "color": "#fff3d4",
                        "intensity": 2,
                        "shadow": True,
                    },
                }
            ],
        },
        {
            "stable_id": "cube",
            "name": "Cube",
            "layer": "default",
            "parent": None,
            "tags": ["mesh"],
            "transform": {
                "x": 0,
                "y": 0,
                "z": 0,
                "rotation_x": 0,
                "rotation_y": 0,
                "rotation_z": 0,
                "scale_x": 1,
                "scale_y": 1,
                "scale_z": 1,
            },
            "components": [
                {
                    "kind": "cube3d",
                    "asset": None,
                    "script": None,
                    "profile": None,
                    "properties": {"color": "#f5a524"},
                }
            ],
        },
    ],
}


def log(message: str) -> None:
    print(f"[borealis-sdk] {message}")


def require_path(path: Path, label: str) -> None:
    if not path.exists():
        raise RuntimeError(f"{label} not found: {path}")


def ensure_inside(child: Path, parent: Path) -> None:
    child_resolved = child.resolve()
    parent_resolved = parent.resolve()
    if child_resolved != parent_resolved and parent_resolved not in child_resolved.parents:
        raise RuntimeError(f"unsafe target outside expected root: {child}")


def remove_managed_entries(sdk_dir: Path) -> None:
    ensure_inside(sdk_dir, STUDIO_ROOT)
    sdk_dir.mkdir(parents=True, exist_ok=True)
    for name in MANAGED_NAMES:
        target = sdk_dir / name
        if target.is_dir():
            shutil.rmtree(target)
        elif target.exists():
            target.unlink()


def copy_tree(src: Path, dst: Path) -> None:
    require_path(src, f"source folder {src.name}")
    shutil.copytree(
        src,
        dst,
        ignore=shutil.ignore_patterns(
            ".git",
            ".ztc-tmp",
            "__pycache__",
            "build",
            "target",
            "*.o",
            "*.obj",
            "*.pdb",
        ),
    )


def copy_optional_file(src: Path, dst: Path) -> bool:
    if not src.exists():
        return False
    dst.parent.mkdir(parents=True, exist_ok=True)
    shutil.copy2(src, dst)
    return True


def run(command: list[str], cwd: Path) -> subprocess.CompletedProcess[str]:
    log("run: " + " ".join(command))
    env = os.environ.copy()
    env["ZENITH_HOME"] = str(cwd)
    result = subprocess.run(
        command,
        cwd=cwd,
        env=env,
        text=True,
        encoding="utf-8",
        errors="replace",
        capture_output=True,
    )
    if result.stdout:
        print(result.stdout)
    if result.stderr:
        print(result.stderr, file=sys.stderr)
    if result.returncode != 0:
        raise RuntimeError(f"command failed with exit code {result.returncode}")
    return result


def write_text(path: Path, content: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(content, encoding="utf-8", newline="\n")


def write_json(path: Path, value: object) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(value, indent=2) + "\n", encoding="utf-8", newline="\n")


def build_zt_if_needed(repo_root: Path, build_zt: bool) -> None:
    zt = repo_root / "zt.exe"
    if zt.exists():
        return
    if not build_zt:
        raise RuntimeError("zt.exe not found. Run python build.py first or pass --build-zt.")
    run([sys.executable, "build.py"], cwd=repo_root)
    require_path(zt, "zt.exe after build")


def assemble(args: argparse.Namespace) -> Path:
    repo_root = Path(args.repo_root).resolve()
    sdk_dir = Path(args.sdk_dir).resolve()

    require_path(repo_root / "zenith.ztproj", "repo zenith.ztproj")
    require_path(repo_root / "packages" / "borealis" / "zenith.ztproj", "Borealis package")
    require_path(repo_root / "packages" / "borealis" / "borealis.editor.json", "Borealis editor manifest")
    require_path(repo_root / "runtime", "Zenith runtime")
    require_path(repo_root / "stdlib", "Zenith stdlib")
    build_zt_if_needed(repo_root, args.build_zt)

    log(f"repo: {repo_root}")
    log(f"sdk:  {sdk_dir}")
    remove_managed_entries(sdk_dir)

    bin_dir = sdk_dir / "bin"
    bin_dir.mkdir(parents=True, exist_ok=True)
    shutil.copy2(repo_root / "zt.exe", bin_dir / "zt.exe")
    copy_optional_file(repo_root / "zpm.exe", bin_dir / "zpm.exe")
    copy_optional_file(repo_root / "zt-lsp.exe", bin_dir / "zt-lsp.exe")

    copy_tree(repo_root / "runtime", sdk_dir / "runtime")
    copy_tree(repo_root / "stdlib", sdk_dir / "stdlib")
    copy_tree(repo_root / "packages" / "borealis", sdk_dir / "packages" / "borealis")

    manifest_src = repo_root / "packages" / "borealis" / "borealis.editor.json"
    shutil.copy2(manifest_src, sdk_dir / "borealis.editor.json")
    (sdk_dir / "editor").mkdir(parents=True, exist_ok=True)
    shutil.copy2(manifest_src, sdk_dir / "editor" / "borealis.editor.json")

    write_text(sdk_dir / "preview" / "zenith.ztproj", PREVIEW_PROJECT)
    write_text(sdk_dir / "preview" / "src" / "app" / "main.zt", PREVIEW_MAIN)
    write_json(sdk_dir / "templates" / "templates.json", [TEMPLATE_3D, TEMPLATE_2D, TEMPLATE_SCRIPTED])
    write_json(sdk_dir / "templates" / "starter-3d.scene.json", STARTER_3D_SCENE)
    write_text(
        sdk_dir / "templates" / "starter-main.zt",
        "namespace app.main\n\npublic func update(entity_id: text, dt: float) -> result<void, core.Error>\n    return success()\nend\n",
    )
    write_text(sdk_dir / "templates" / "triangle.obj", "o Triangle\nv 0 0 0\nv 1 0 0\nv 0 1 0\nf 1 2 3\n")
    write_json(
        sdk_dir / "sdk-manifest.json",
        {
            "name": "borealis-studio-sdk",
            "schemaVersion": 1,
            "generatedAtUtc": dt.datetime.now(dt.timezone.utc).isoformat(),
            "sourceRepo": str(repo_root),
            "layout": {
                "zt": "bin/zt.exe",
                "borealisPackage": "packages/borealis",
                "editorManifest": "borealis.editor.json",
                "previewProject": "preview/zenith.ztproj",
                "templates": "templates/templates.json",
            },
        },
    )

    if not args.skip_check:
        zt_exe = sdk_dir / "bin" / "zt.exe"
        run([str(zt_exe), "check", str(sdk_dir / "preview" / "zenith.ztproj")], cwd=sdk_dir)
        run([str(zt_exe), "check", str(sdk_dir / "packages" / "borealis" / "zenith.ztproj"), "--all"], cwd=sdk_dir)
        if not args.skip_smoke:
            run_preview_smoke(sdk_dir)

    log("SDK assembled.")
    return sdk_dir


def run_preview_smoke(sdk_dir: Path) -> None:
    zt_exe = sdk_dir / "bin" / "zt.exe"
    preview_project = sdk_dir / "preview" / "zenith.ztproj"
    input_lines = "\n".join(
        [
            '{"protocol":1,"seq":1,"channel":"command","kind":"hello","payload":{"role":"studio"}}',
            '{"protocol":1,"seq":2,"channel":"command","kind":"open_project","payload":{"path":"preview/zenith.ztproj"}}',
            '{"protocol":1,"seq":3,"channel":"command","kind":"open_scene","payload":{"path":"templates/starter-3d.scene.json"}}',
            '{"protocol":1,"seq":4,"channel":"command","kind":"enter_play_mode","payload":{"scene":"preview.scene.json"}}',
            '{"protocol":1,"seq":5,"channel":"command","kind":"stop_play_mode","payload":{}}',
            "",
        ]
    )
    env = os.environ.copy()
    env["ZENITH_HOME"] = str(sdk_dir)
    command = [str(zt_exe), "run", str(preview_project)]
    log("run: " + " ".join(command) + " < preview smoke")
    result = subprocess.run(
        command,
        cwd=sdk_dir,
        env=env,
        input=input_lines,
        text=True,
        encoding="utf-8",
        errors="replace",
        capture_output=True,
    )
    if result.stdout:
        print(result.stdout)
    if result.stderr:
        print(result.stderr, file=sys.stderr)
    if result.returncode != 0:
        raise RuntimeError(f"preview smoke failed with exit code {result.returncode}")
    if '"status":"playing"' not in result.stdout or '"status":"stopped"' not in result.stdout:
        raise RuntimeError("preview smoke did not emit playing/stopped status")


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Assemble Borealis Studio runtime SDK.")
    parser.add_argument("--repo-root", default=str(DEFAULT_REPO_ROOT))
    parser.add_argument("--sdk-dir", default=str(DEFAULT_SDK_DIR))
    parser.add_argument("--build-zt", action="store_true", help="Run build.py if zt.exe is missing.")
    parser.add_argument("--skip-check", action="store_true", help="Skip zt check validation.")
    parser.add_argument("--skip-smoke", action="store_true", help="Skip preview JSONL smoke run.")
    return parser.parse_args()


def main() -> int:
    try:
        assemble(parse_args())
        return 0
    except Exception as error:
        print(f"[borealis-sdk] ERROR: {error}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
