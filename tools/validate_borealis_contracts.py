from __future__ import annotations

import json
import re
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
BOREALIS = ROOT / "packages" / "borealis"
SRC = BOREALIS / "src" / "borealis"
MANIFEST = BOREALIS / "borealis.editor.json"
SCENES = [
    BOREALIS / "scenes" / "sample.scene.json",
    BOREALIS / "scenes" / "sample_3d.scene.json",
]

PUBLIC_HELPER_PATTERNS = [
    re.compile(r"^public func .*(?:_or\()"),
    re.compile(r"^public func .*_set\(values: map"),
    re.compile(r"^public func set_.*\(values: map"),
    re.compile(r"^public func .*_internal\("),
    re.compile(r"^public func (?:require_|.*_require_exists)"),
    re.compile(r"^public func normalize_"),
    re.compile(r"^public func profile_value_key\("),
    re.compile(r"^public func emit_internal\("),
]


def fail(errors: list[str], message: str) -> None:
    errors.append(message)


def load_json(path: Path, errors: list[str]) -> object:
    try:
        return json.loads(path.read_text(encoding="utf-8"))
    except FileNotFoundError:
        fail(errors, f"missing file: {path.relative_to(ROOT)}")
    except json.JSONDecodeError as exc:
        fail(errors, f"invalid json: {path.relative_to(ROOT)}: {exc}")
    return {}


def check_public_surface(errors: list[str]) -> None:
    for path in sorted(SRC.rglob("*.zt")):
        rel = path.relative_to(ROOT)
        for line_no, line in enumerate(path.read_text(encoding="utf-8").splitlines(), start=1):
            stripped = line.strip()
            if stripped.startswith("public var "):
                fail(errors, f"{rel}:{line_no}: public module state is not allowed")
            for pattern in PUBLIC_HELPER_PATTERNS:
                if pattern.search(stripped):
                    fail(errors, f"{rel}:{line_no}: internal helper is public: {stripped}")


def check_scene(path: Path, manifest: dict, errors: list[str]) -> None:
    scene = load_json(path, errors)
    if not isinstance(scene, dict):
        fail(errors, f"{path.relative_to(ROOT)}: scene root must be an object")
        return

    rel = path.relative_to(ROOT)
    if scene.get("version") != 2:
        fail(errors, f"{rel}: version must be 2")

    for key in ["name", "document_id", "environment", "render", "audio", "entities"]:
        if key not in scene:
            fail(errors, f"{rel}: missing top-level key '{key}'")

    environment = scene.get("environment")
    if isinstance(environment, dict):
        for key in ["skybox", "ambient", "fog", "weather"]:
            if key not in environment:
                fail(errors, f"{rel}: environment missing '{key}'")
    else:
        fail(errors, f"{rel}: environment must be an object")

    render = scene.get("render")
    if isinstance(render, dict):
        if "postfx" not in render:
            fail(errors, f"{rel}: render missing 'postfx'")
    else:
        fail(errors, f"{rel}: render must be an object")

    components = manifest.get("components", {})
    if not isinstance(components, dict):
        fail(errors, "borealis.editor.json: components must be an object")
        components = {}

    entities = scene.get("entities")
    if not isinstance(entities, list):
        fail(errors, f"{rel}: entities must be a list")
        return

    for index, entity in enumerate(entities):
        if not isinstance(entity, dict):
            fail(errors, f"{rel}: entities[{index}] must be an object")
            continue
        for key in ["stable_id", "name", "layer", "tags", "transform", "components"]:
            if key not in entity:
                fail(errors, f"{rel}: entities[{index}] missing '{key}'")
        entity_components = entity.get("components", [])
        if not isinstance(entity_components, list):
            fail(errors, f"{rel}: entities[{index}].components must be a list")
            continue
        for comp_index, component in enumerate(entity_components):
            if not isinstance(component, dict):
                fail(errors, f"{rel}: entities[{index}].components[{comp_index}] must be an object")
                continue
            kind = component.get("kind")
            if kind not in components:
                fail(errors, f"{rel}: unknown component kind '{kind}'")
            if "properties" not in component:
                fail(errors, f"{rel}: component '{kind}' missing properties object")


def check_manifest(manifest: dict, errors: list[str]) -> None:
    scene_settings = manifest.get("sceneSettings", {})
    if not isinstance(scene_settings, dict):
        fail(errors, "borealis.editor.json: sceneSettings must be an object")
        return
    for key in ["skybox", "ambient", "fog", "weather", "postfx"]:
        if key not in scene_settings:
            fail(errors, f"borealis.editor.json: sceneSettings missing '{key}'")


def main() -> int:
    errors: list[str] = []
    manifest = load_json(MANIFEST, errors)
    if not isinstance(manifest, dict):
        fail(errors, "borealis.editor.json root must be an object")
        manifest = {}

    check_public_surface(errors)
    check_manifest(manifest, errors)
    for scene in SCENES:
        check_scene(scene, manifest, errors)

    if errors:
        for error in errors:
            print(f"ERROR: {error}")
        return 1

    print("borealis contract validation ok")
    return 0


if __name__ == "__main__":
    sys.exit(main())
