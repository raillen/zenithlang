from __future__ import annotations

import json
from pathlib import Path
from typing import Any

import numpy as np


def load_store(metadata_path: Path, vectors_path: Path) -> tuple[list[dict[str, Any]], np.ndarray]:
    if not metadata_path.exists() or not vectors_path.exists():
        return [], np.zeros((0, 0), dtype=np.float32)

    rows = json.loads(metadata_path.read_text(encoding="utf-8"))
    vectors = np.load(vectors_path)
    if len(rows) != len(vectors):
        return [], np.zeros((0, 0), dtype=np.float32)
    return rows, vectors.astype(np.float32, copy=False)


def save_store(
    metadata_path: Path,
    vectors_path: Path,
    rows: list[dict[str, Any]],
    vectors: np.ndarray,
) -> None:
    metadata_path.parent.mkdir(parents=True, exist_ok=True)
    vectors_path.parent.mkdir(parents=True, exist_ok=True)
    metadata_path.write_text(
        json.dumps(rows, ensure_ascii=True, separators=(",", ":")),
        encoding="utf-8",
    )
    np.save(vectors_path, vectors.astype(np.float32, copy=False))


def empty_vectors(dimensions: int = 0) -> np.ndarray:
    return np.zeros((0, dimensions), dtype=np.float32)
