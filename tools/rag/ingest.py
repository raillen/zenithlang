from __future__ import annotations

import argparse
import os
import sys
from pathlib import Path

import numpy as np


THIS_DIR = Path(__file__).resolve().parent
if str(THIS_DIR) not in sys.path:
    sys.path.insert(0, str(THIS_DIR))

from chunking import build_chunks_for_path
from common import (  # noqa: E402
    DEFAULT_SOURCE_ROOTS,
    DEFAULT_PROVIDER,
    ProviderAPIError,
    batched,
    clear_legacy_chroma_cache,
    collection_name_for_embedding_model,
    create_provider_client,
    default_manifest,
    default_base_url_for_provider,
    default_embedding_model_for_provider,
    detect_incompatible_chroma_collection,
    ensure_rag_dirs,
    iter_source_files,
    load_manifest,
    manifest_path_for_embedding_model,
    normalize_provider,
    provider_label,
    read_text_file,
    relative_path,
    safe_print,
    save_manifest,
    sha256_text,
    store_metadata_path_for_embedding_model,
    store_vectors_path_for_embedding_model,
    utc_timestamp,
)
from store import empty_vectors, load_store, save_store  # noqa: E402


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Build or update the local Zenith RAG index."
    )
    parser.add_argument(
        "--provider",
        default=DEFAULT_PROVIDER,
        choices=("ollama", "lmstudio"),
        help="Backend used for embeddings. Default: %(default)s",
    )
    parser.add_argument(
        "--embedding-model",
        default=None,
        help="Embedding model served by the selected provider.",
    )
    parser.add_argument(
        "--base-url",
        default=None,
        help="Base URL for the local provider server.",
    )
    parser.add_argument(
        "--api-key",
        default=os.environ.get("LMSTUDIO_API_KEY"),
        help="Optional API key. Useful for LM Studio if you enabled auth.",
    )
    parser.add_argument(
        "--collection-prefix",
        default="zenith",
        help="Prefix used in the local store file names.",
    )
    parser.add_argument(
        "--roots",
        nargs="*",
        default=None,
        help=(
            "Optional list of repo-relative roots to index. "
            f"Default roots: {', '.join(DEFAULT_SOURCE_ROOTS)}"
        ),
    )
    parser.add_argument(
        "--limit",
        type=int,
        default=0,
        help="Index only the first N files. Useful for quick tests.",
    )
    parser.add_argument(
        "--batch-size",
        type=int,
        default=24,
        help="How many chunks to embed per provider request. Default: %(default)s",
    )
    parser.add_argument(
        "--rebuild",
        action="store_true",
        help="Rebuild the local vector store from scratch.",
    )
    return parser.parse_args()


def remove_chunk_ids(
    rows: list[dict[str, object]],
    vectors: np.ndarray,
    chunk_ids: set[str],
) -> tuple[list[dict[str, object]], np.ndarray]:
    if not chunk_ids or not rows:
        return rows, vectors

    keep_indices = [
        index
        for index, row in enumerate(rows)
        if str(row.get("id", "")) not in chunk_ids
    ]
    filtered_rows = [rows[index] for index in keep_indices]
    if vectors.size == 0:
        return filtered_rows, vectors
    if not keep_indices:
        return filtered_rows, empty_vectors(vectors.shape[1])
    return filtered_rows, vectors[keep_indices]


def main() -> int:
    args = parse_args()
    ensure_rag_dirs()
    provider = normalize_provider(args.provider)
    embedding_model = args.embedding_model or default_embedding_model_for_provider(provider)
    base_url = args.base_url or default_base_url_for_provider(provider)

    collection_name = collection_name_for_embedding_model(
        provider,
        embedding_model,
        prefix=args.collection_prefix,
    )
    manifest_path = manifest_path_for_embedding_model(
        provider,
        embedding_model,
        prefix=args.collection_prefix,
    )
    metadata_path = store_metadata_path_for_embedding_model(
        provider,
        embedding_model,
        prefix=args.collection_prefix,
    )
    vectors_path = store_vectors_path_for_embedding_model(
        provider,
        embedding_model,
        prefix=args.collection_prefix,
    )

    incompatibility_reason = detect_incompatible_chroma_collection(collection_name)
    if incompatibility_reason:
        safe_print("Detected an incompatible legacy Chroma cache.")
        safe_print(f"- Reason: {incompatibility_reason}")
        if clear_legacy_chroma_cache():
            safe_print("- Action: removed .rag/chroma and continuing with the simple local store.")
    elif clear_legacy_chroma_cache():
        safe_print("Removed an unused legacy .rag/chroma cache.")

    if args.rebuild:
        manifest = default_manifest(collection_name, embedding_model, provider)
        rows: list[dict[str, object]] = []
        vectors = empty_vectors()
    else:
        manifest = load_manifest(
            manifest_path,
            collection_name,
            embedding_model,
            provider,
        )
        rows, vectors = load_store(metadata_path, vectors_path)
        if manifest.get("files") and not rows:
            safe_print("Detected a stale manifest with an empty local vector store.")
            safe_print("- Action: reindexing all files for this embedding model.")
            manifest = default_manifest(collection_name, embedding_model, provider)

    client = create_provider_client(
        provider,
        base_url=base_url,
        api_key=args.api_key,
    )

    files = iter_source_files(args.roots)
    if args.limit > 0:
        files = files[: args.limit]

    current_paths = {relative_path(path): path for path in files}
    manifest_files = manifest.setdefault("files", {})

    deleted_files = sorted(set(manifest_files) - set(current_paths))
    for rel_path in deleted_files:
        chunk_ids = set(manifest_files[rel_path].get("ids", []))
        rows, vectors = remove_chunk_ids(rows, vectors, chunk_ids)
        del manifest_files[rel_path]
        safe_print(f"[delete] {rel_path}")

    indexed_files = 0
    skipped_files = 0
    chunk_count = 0

    for path in files:
        rel_path = relative_path(path)
        text = read_text_file(path)
        file_hash = sha256_text(text)
        existing = manifest_files.get(rel_path)

        if existing and existing.get("sha256") == file_hash:
            skipped_files += 1
            continue

        if existing:
            rows, vectors = remove_chunk_ids(rows, vectors, set(existing.get("ids", [])))

        chunks = build_chunks_for_path(path, text)
        if not chunks:
            manifest_files.pop(rel_path, None)
            safe_print(f"[skip]   {rel_path} (no supported chunks)")
            continue

        new_rows: list[dict[str, object]] = []
        new_vector_batches: list[np.ndarray] = []

        for chunk_batch in batched(chunks, args.batch_size):
            embedded = client.embed_texts(
                embedding_model,
                [chunk.text for chunk in chunk_batch],
            )
            new_vector_batches.append(np.asarray(embedded, dtype=np.float32))
            for chunk in chunk_batch:
                new_rows.append(
                    {
                        "id": chunk.chunk_id,
                        "document": chunk.text,
                        "metadata": chunk.metadata,
                    }
                )

        if new_rows:
            new_vectors = np.vstack(new_vector_batches)
            if vectors.size == 0:
                vectors = new_vectors
            else:
                vectors = np.vstack([vectors, new_vectors])
            rows.extend(new_rows)

        manifest_files[rel_path] = {
            "ids": [chunk.chunk_id for chunk in chunks],
            "chunk_count": len(chunks),
            "sha256": file_hash,
            "updated_at": utc_timestamp(),
        }
        indexed_files += 1
        chunk_count += len(chunks)
        safe_print(f"[index]  {rel_path} ({len(chunks)} chunks)")

    save_store(metadata_path, vectors_path, rows, vectors)
    save_manifest(manifest_path, manifest)
    safe_print("")
    safe_print("RAG index ready.")
    safe_print(f"- Store: {collection_name}")
    safe_print(f"- Provider: {provider_label(provider)}")
    safe_print(f"- Base URL: {base_url}")
    safe_print(f"- Embedding model: {embedding_model}")
    safe_print(f"- Indexed files this run: {indexed_files}")
    safe_print(f"- Unchanged files skipped: {skipped_files}")
    safe_print(f"- Deleted files removed: {len(deleted_files)}")
    safe_print(f"- New or refreshed chunks: {chunk_count}")
    safe_print(f"- Metadata file: {metadata_path}")
    safe_print(f"- Vectors file: {vectors_path}")
    safe_print(f"- Manifest: {manifest_path}")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except ProviderAPIError as exc:
        raise SystemExit(f"Provider error: {exc}") from exc
