from __future__ import annotations

import fnmatch
import hashlib
import json
import os
import re
import shutil
import sqlite3
import urllib.error
import urllib.request
from datetime import datetime, timezone
from pathlib import Path
from typing import Any, Iterable, Iterator, Sequence


ROOT = Path(__file__).resolve().parents[2]
RAG_DIR = ROOT / ".rag"
CHROMA_DIR = RAG_DIR / "chroma"
DEFAULT_PROVIDER = os.environ.get("ZENITH_RAG_PROVIDER", "ollama")
DEFAULT_OLLAMA_URL = os.environ.get("OLLAMA_BASE_URL", "http://localhost:11434")
DEFAULT_LMSTUDIO_URL = os.environ.get("LMSTUDIO_BASE_URL", "http://localhost:1234/v1")
DEFAULT_OLLAMA_EMBEDDING_MODEL = "embeddinggemma"
DEFAULT_OLLAMA_CHAT_MODEL = "qwen2.5-coder:3b"
DEFAULT_LMSTUDIO_EMBEDDING_MODEL = os.environ.get(
    "LMSTUDIO_EMBEDDING_MODEL",
    "nomic-embed-text-v1.5",
)
DEFAULT_LMSTUDIO_CHAT_MODEL = os.environ.get(
    "LMSTUDIO_CHAT_MODEL",
    "qwen2.5-coder-3b-instruct",
)
DEFAULT_EMBEDDING_MODEL = DEFAULT_OLLAMA_EMBEDDING_MODEL
DEFAULT_CHAT_MODEL = DEFAULT_OLLAMA_CHAT_MODEL
PROVIDER_CHOICES = ("ollama", "lmstudio")
DEFAULT_SOURCE_ROOTS = (
    "README.md",
    "language",
    "compiler",
    "runtime/c",
    "stdlib",
    "examples",
    "tests",
)
ALLOWED_EXTENSIONS = {".c", ".h", ".md", ".toml", ".txt", ".zdoc", ".zt"}
DOC_EXTENSIONS = {".md", ".toml", ".txt", ".zdoc"}
CODE_EXTENSIONS = {".c", ".h", ".zt"}
SKIP_DIR_NAMES = {
    ".git",
    ".github",
    ".idea",
    ".ruff_cache",
    ".vscode",
    "__pycache__",
    "build",
    "coverage",
    "dist",
    "preview",
    "reports",
    "target",
}
SKIP_GLOBS = (
    "*.exe",
    "*.gcov",
    "*.json",
    "*.log",
    "*.png",
    "*.svg",
    "*.zip",
    "docs/reports/*",
    "packages/*/build/*",
    "tests/heavy/fuzz/*/src/fuzz/*",
    "tools/borealis-editor/target/*",
)
PRIORITY_RULES = (
    ("language/spec/", ("spec", 100)),
    ("language/decisions/", ("decision", 95)),
    ("language/", ("language", 85)),
    ("README.md", ("overview", 82)),
    ("compiler/", ("code", 75)),
    ("runtime/c/", ("runtime", 72)),
    ("stdlib/", ("stdlib", 64)),
    ("examples/", ("example", 58)),
    ("tests/", ("test", 52)),
)


class ProviderAPIError(RuntimeError):
    pass


OllamaAPIError = ProviderAPIError


def safe_print(message: object = "") -> None:
    try:
        print(message)
    except UnicodeEncodeError:
        text = str(message)
        encoding = os.device_encoding(1) or "utf-8"
        sanitized = text.encode(encoding, errors="replace").decode(
            encoding, errors="replace"
        )
        print(sanitized)


def ensure_rag_dirs() -> None:
    RAG_DIR.mkdir(parents=True, exist_ok=True)


def normalize_ollama_url(base_url: str) -> str:
    cleaned = base_url.strip().rstrip("/")
    if cleaned.endswith("/api"):
        return cleaned[:-4]
    return cleaned


def normalize_lmstudio_url(base_url: str) -> str:
    cleaned = base_url.strip().rstrip("/")
    if cleaned.endswith("/api/v1"):
        return cleaned[:-4]
    if not cleaned.endswith("/v1"):
        return cleaned + "/v1"
    return cleaned


def normalize_provider(provider: str) -> str:
    cleaned = provider.strip().lower()
    if cleaned not in PROVIDER_CHOICES:
        raise ValueError(
            f"Unsupported provider '{provider}'. Choose one of: {', '.join(PROVIDER_CHOICES)}."
        )
    return cleaned


def provider_label(provider: str) -> str:
    if normalize_provider(provider) == "lmstudio":
        return "LM Studio"
    return "Ollama"


def default_base_url_for_provider(provider: str) -> str:
    normalized = normalize_provider(provider)
    if normalized == "lmstudio":
        return DEFAULT_LMSTUDIO_URL
    return DEFAULT_OLLAMA_URL


def default_embedding_model_for_provider(provider: str) -> str:
    normalized = normalize_provider(provider)
    if normalized == "lmstudio":
        return DEFAULT_LMSTUDIO_EMBEDDING_MODEL
    return DEFAULT_OLLAMA_EMBEDDING_MODEL


def default_chat_model_for_provider(provider: str) -> str:
    normalized = normalize_provider(provider)
    if normalized == "lmstudio":
        return DEFAULT_LMSTUDIO_CHAT_MODEL
    return DEFAULT_OLLAMA_CHAT_MODEL


def slugify_model_name(model_name: str) -> str:
    return re.sub(r"[^a-z0-9]+", "_", model_name.lower()).strip("_")


def collection_name_for_embedding_model(
    provider: str,
    embedding_model: str,
    prefix: str = "zenith",
) -> str:
    normalized = normalize_provider(provider)
    suffix = slugify_model_name(embedding_model)
    if normalized != "ollama":
        suffix = f"{normalized}_{suffix}"
    return f"{prefix}_{suffix}"


def manifest_path_for_embedding_model(
    provider: str,
    embedding_model: str,
    prefix: str = "zenith",
) -> Path:
    return RAG_DIR / f"{collection_name_for_embedding_model(provider, embedding_model, prefix)}_manifest.json"


def store_metadata_path_for_embedding_model(
    provider: str,
    embedding_model: str,
    prefix: str = "zenith",
) -> Path:
    return RAG_DIR / f"{collection_name_for_embedding_model(provider, embedding_model, prefix)}_store.json"


def store_vectors_path_for_embedding_model(
    provider: str,
    embedding_model: str,
    prefix: str = "zenith",
) -> Path:
    return RAG_DIR / f"{collection_name_for_embedding_model(provider, embedding_model, prefix)}_vectors.npy"


def utc_timestamp() -> str:
    return datetime.now(timezone.utc).replace(microsecond=0).isoformat()


def relative_path(path: Path) -> str:
    return path.resolve().relative_to(ROOT).as_posix()


def classify_path(rel_path: str) -> tuple[str, int]:
    for prefix, result in PRIORITY_RULES:
        if rel_path == prefix or rel_path.startswith(prefix):
            return result
    return "other", 40


def read_text_file(path: Path) -> str:
    return path.read_text(encoding="utf-8", errors="replace")


def sha256_text(text: str) -> str:
    return hashlib.sha256(text.encode("utf-8")).hexdigest()


def batched(items: Sequence[Any], batch_size: int) -> Iterator[Sequence[Any]]:
    if batch_size <= 0:
        raise ValueError("batch_size must be greater than 0")

    for index in range(0, len(items), batch_size):
        yield items[index : index + batch_size]


def default_manifest(
    collection_name: str,
    embedding_model: str,
    provider: str = "ollama",
) -> dict[str, Any]:
    return {
        "collection_name": collection_name,
        "embedding_model": embedding_model,
        "provider": normalize_provider(provider),
        "files": {},
        "updated_at": None,
    }


def load_manifest(
    path: Path,
    collection_name: str,
    embedding_model: str,
    provider: str = "ollama",
) -> dict[str, Any]:
    normalized_provider = normalize_provider(provider)
    if not path.exists():
        return default_manifest(collection_name, embedding_model, normalized_provider)

    try:
        payload = json.loads(path.read_text(encoding="utf-8"))
    except json.JSONDecodeError:
        return default_manifest(collection_name, embedding_model, normalized_provider)

    if payload.get("collection_name") != collection_name:
        return default_manifest(collection_name, embedding_model, normalized_provider)

    stored_provider = payload.get("provider")
    if stored_provider and stored_provider != normalized_provider:
        return default_manifest(collection_name, embedding_model, normalized_provider)
    if stored_provider is None and normalized_provider != "ollama":
        return default_manifest(collection_name, embedding_model, normalized_provider)

    payload.setdefault("files", {})
    payload["embedding_model"] = embedding_model
    payload["collection_name"] = collection_name
    payload["provider"] = normalized_provider
    return payload


def save_manifest(path: Path, payload: dict[str, Any]) -> None:
    payload["updated_at"] = utc_timestamp()
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(payload, indent=2, sort_keys=True), encoding="utf-8")

def clear_legacy_chroma_cache() -> bool:
    if not CHROMA_DIR.exists():
        return False
    shutil.rmtree(CHROMA_DIR, ignore_errors=True)
    return True


def detect_incompatible_chroma_collection(collection_name: str) -> str | None:
    db_path = CHROMA_DIR / "chroma.sqlite3"
    if not db_path.exists():
        return None

    try:
        connection = sqlite3.connect(str(db_path))
    except sqlite3.Error as exc:
        return f"could not open Chroma sqlite database: {exc}"

    try:
        cursor = connection.cursor()
        tables = {
            row[0]
            for row in cursor.execute(
                "SELECT name FROM sqlite_master WHERE type='table'"
            ).fetchall()
        }
        if "collections" not in tables:
            return None

        rows = cursor.execute(
            "SELECT config_json_str FROM collections WHERE name = ?",
            (collection_name,),
        ).fetchall()
    except sqlite3.Error as exc:
        return f"could not inspect collection metadata: {exc}"
    finally:
        connection.close()

    for (config_json_str,) in rows:
        if not config_json_str or config_json_str in {"{}", "null"}:
            return "stored collection config is empty"

        try:
            payload = json.loads(config_json_str)
        except json.JSONDecodeError:
            return "stored collection config is invalid JSON"

        if not isinstance(payload, dict) or "_type" not in payload:
            return "stored collection config is missing the _type field"

    return None


def should_index_file(path: Path) -> bool:
    if not path.is_file():
        return False

    rel_path = relative_path(path)
    if path.suffix.lower() not in ALLOWED_EXTENSIONS:
        return False

    rel_parts = Path(rel_path).parts
    if any(part in SKIP_DIR_NAMES for part in rel_parts):
        return False

    for pattern in SKIP_GLOBS:
        if fnmatch.fnmatch(rel_path, pattern):
            return False

    return True


def iter_source_files(roots: Iterable[str] | None = None) -> list[Path]:
    selected_roots = tuple(roots or DEFAULT_SOURCE_ROOTS)
    results: list[Path] = []

    for rel_root in selected_roots:
        candidate = (ROOT / rel_root).resolve()
        if not candidate.exists():
            continue

        if candidate.is_file():
            if should_index_file(candidate):
                results.append(candidate)
            continue

        for path in candidate.rglob("*"):
            if should_index_file(path):
                results.append(path)

    results.sort(key=lambda item: relative_path(item).lower())
    return results


def format_source_label(metadata: dict[str, Any]) -> str:
    rel_path = metadata.get("path", "?")
    start = metadata.get("start_line", "?")
    end = metadata.get("end_line", "?")
    kind = metadata.get("kind", "other")
    return f"{rel_path}:{start}-{end} [{kind}]"


def _read_error_detail(raw_detail: str) -> str:
    detail_text = raw_detail
    try:
        detail_payload = json.loads(raw_detail)
    except json.JSONDecodeError:
        return detail_text

    if isinstance(detail_payload, dict):
        error = detail_payload.get("error")
        if isinstance(error, dict):
            return str(error.get("message") or error)
        if error is not None:
            return str(error)
        message = detail_payload.get("message")
        if message is not None:
            return str(message)
    return detail_text


class JSONProviderClient:
    def __init__(
        self,
        provider: str,
        base_url: str,
        *,
        api_key: str | None = None,
    ) -> None:
        self.provider = normalize_provider(provider)
        self.base_url = base_url.rstrip("/")
        self.api_key = api_key

    def _request_headers(self) -> dict[str, str]:
        headers = {"Content-Type": "application/json"}
        if self.api_key:
            headers["Authorization"] = f"Bearer {self.api_key}"
        return headers

    def _post_json(self, endpoint: str, payload: dict[str, Any]) -> dict[str, Any]:
        data = json.dumps(payload).encode("utf-8")
        request = urllib.request.Request(
            f"{self.base_url}/{endpoint.lstrip('/')}",
            data=data,
            headers=self._request_headers(),
            method="POST",
        )

        try:
            with urllib.request.urlopen(request, timeout=180) as response:
                body = response.read().decode("utf-8")
        except urllib.error.HTTPError as exc:
            detail_text = _read_error_detail(exc.read().decode("utf-8", errors="replace"))
            raise ProviderAPIError(
                f"{provider_label(self.provider)} returned HTTP {exc.code}: {detail_text}"
            ) from exc
        except urllib.error.URLError as exc:
            raise ProviderAPIError(
                f"Could not reach {provider_label(self.provider)} on {self.base_url}."
            ) from exc

        try:
            return json.loads(body)
        except json.JSONDecodeError as exc:
            raise ProviderAPIError(
                f"{provider_label(self.provider)} returned invalid JSON: {body[:200]}"
            ) from exc


class OllamaClient:
    def __init__(self, base_url: str = DEFAULT_OLLAMA_URL) -> None:
        self.provider = "ollama"
        self.base_url = normalize_ollama_url(base_url)
        self._client = JSONProviderClient("ollama", f"{self.base_url}/api")

    def _post_json(self, endpoint: str, payload: dict[str, Any]) -> dict[str, Any]:
        try:
            return self._client._post_json(endpoint, payload)
        except ProviderAPIError as exc:
            detail_text = str(exc)
            model_match = re.search(r'model "([^"]+)" not found', detail_text)
            if model_match:
                detail = (
                    f"{detail_text}\nRun: ollama pull {model_match.group(1)}"
                )
            else:
                detail = detail_text
            raise ProviderAPIError(detail) from exc

    def embed_texts(self, model: str, texts: Sequence[str]) -> list[list[float]]:
        if not texts:
            return []

        payload = {
            "model": model,
            "input": list(texts),
            "truncate": True,
        }
        result = self._post_json("embed", payload)
        embeddings = result.get("embeddings")
        if not isinstance(embeddings, list):
            raise ProviderAPIError("Ollama /api/embed returned no embeddings.")
        return embeddings

    def chat(
        self,
        model: str,
        system_prompt: str,
        user_prompt: str,
        *,
        temperature: float,
        num_ctx: int,
        num_predict: int,
        keep_alive: str = "5m",
    ) -> dict[str, Any]:
        payload = {
            "model": model,
            "messages": [
                {"role": "system", "content": system_prompt},
                {"role": "user", "content": user_prompt},
            ],
            "stream": False,
            "keep_alive": keep_alive,
            "options": {
                "temperature": temperature,
                "num_ctx": num_ctx,
                "num_predict": num_predict,
            },
        }
        return self._post_json("chat", payload)


class LMStudioClient:
    def __init__(
        self,
        base_url: str = DEFAULT_LMSTUDIO_URL,
        *,
        api_key: str | None = None,
    ) -> None:
        self.provider = "lmstudio"
        self.base_url = normalize_lmstudio_url(base_url)
        self._client = JSONProviderClient("lmstudio", self.base_url, api_key=api_key)

    def embed_texts(self, model: str, texts: Sequence[str]) -> list[list[float]]:
        if not texts:
            return []

        payload = {
            "model": model,
            "input": list(texts),
        }
        result = self._client._post_json("embeddings", payload)
        data = result.get("data")
        if not isinstance(data, list):
            raise ProviderAPIError("LM Studio /v1/embeddings returned no data.")

        embeddings: list[list[float]] = []
        for item in data:
            if not isinstance(item, dict) or not isinstance(item.get("embedding"), list):
                raise ProviderAPIError(
                    "LM Studio /v1/embeddings returned an invalid embedding payload."
                )
            embeddings.append(item["embedding"])
        return embeddings

    def chat(
        self,
        model: str,
        system_prompt: str,
        user_prompt: str,
        *,
        temperature: float,
        num_ctx: int,
        num_predict: int,
        keep_alive: str = "5m",
    ) -> dict[str, Any]:
        payload = {
            "model": model,
            "messages": [
                {"role": "system", "content": system_prompt},
                {"role": "user", "content": user_prompt},
            ],
            "temperature": temperature,
            "max_tokens": num_predict,
            "stream": False,
        }
        response = self._client._post_json("chat/completions", payload)
        choices = response.get("choices")
        if not isinstance(choices, list) or not choices:
            raise ProviderAPIError("LM Studio /v1/chat/completions returned no choices.")

        message = choices[0].get("message", {})
        content = message.get("content", "")
        if isinstance(content, list):
            content = "\n".join(
                str(item.get("text", ""))
                for item in content
                if isinstance(item, dict)
            ).strip()
        return {"message": {"content": str(content)}}


def create_provider_client(
    provider: str,
    *,
    base_url: str | None = None,
    api_key: str | None = None,
):
    normalized = normalize_provider(provider)
    if normalized == "lmstudio":
        return LMStudioClient(
            base_url=base_url or DEFAULT_LMSTUDIO_URL,
            api_key=api_key,
        )
    return OllamaClient(base_url=base_url or DEFAULT_OLLAMA_URL)
