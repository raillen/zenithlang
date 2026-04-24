from __future__ import annotations

import argparse
import os
import sys
from pathlib import Path
from typing import Any

import numpy as np


THIS_DIR = Path(__file__).resolve().parent
if str(THIS_DIR) not in sys.path:
    sys.path.insert(0, str(THIS_DIR))

from common import (  # noqa: E402
    DEFAULT_PROVIDER,
    ProviderAPIError,
    clear_legacy_chroma_cache,
    collection_name_for_embedding_model,
    create_provider_client,
    default_base_url_for_provider,
    default_chat_model_for_provider,
    default_embedding_model_for_provider,
    detect_incompatible_chroma_collection,
    ensure_rag_dirs,
    format_source_label,
    manifest_path_for_embedding_model,
    normalize_provider,
    provider_label,
    safe_print,
    store_metadata_path_for_embedding_model,
    store_vectors_path_for_embedding_model,
)
from store import load_store  # noqa: E402


SYSTEM_PROMPT = """You are the local Zenith RAG assistant.

Goals:
- Help with Zenith language design, C compiler/runtime code, tests, and examples.
- Stay grounded in the retrieved repository context.

Rules:
- Use the retrieved context as your main evidence.
- If the evidence is missing or weak, say that clearly.
- Cite file paths when you make concrete claims.
- If sources conflict, prioritize:
  language/spec > language/decisions > compiler/runtime > stdlib > examples/tests.
- Keep the answer direct, practical, and easy to scan.
"""


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Ask questions to the local Zenith RAG assistant."
    )
    parser.add_argument(
        "--provider",
        default=DEFAULT_PROVIDER,
        choices=("ollama", "lmstudio"),
        help="Backend used for embeddings and chat. Default: %(default)s",
    )
    parser.add_argument(
        "question",
        nargs="*",
        help="Question to ask. Leave empty to start an interactive shell.",
    )
    parser.add_argument(
        "--model",
        default=None,
        help="Chat model served by the selected provider.",
    )
    parser.add_argument(
        "--embedding-model",
        default=None,
        help="Embedding model used when the index was built.",
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
        "--top-k",
        type=int,
        default=6,
        help="How many chunks to keep after retrieval. Default: %(default)s",
    )
    parser.add_argument(
        "--fetch-k",
        type=int,
        default=12,
        help="How many raw results to fetch before de-duplicating. Default: %(default)s",
    )
    parser.add_argument(
        "--max-context-chars",
        type=int,
        default=12000,
        help="Soft cap for the combined retrieved context. Default: %(default)s",
    )
    parser.add_argument(
        "--temperature",
        type=float,
        default=0.1,
        help="Sampling temperature sent to the selected provider. Default: %(default)s",
    )
    parser.add_argument(
        "--num-ctx",
        type=int,
        default=6144,
        help="Context hint for providers that support it. Default: %(default)s",
    )
    parser.add_argument(
        "--num-predict",
        type=int,
        default=700,
        help="Maximum answer tokens requested from the selected provider. Default: %(default)s",
    )
    parser.add_argument(
        "--show-context",
        action="store_true",
        help="Print the retrieved context chunks before the answer.",
    )
    return parser.parse_args()


def rerank_results(results: list[dict[str, Any]], top_k: int) -> list[dict[str, Any]]:
    results.sort(
        key=lambda row: (
            -float(row["score"]),
            -int(row["metadata"].get("priority", 0)),
            str(row["metadata"].get("path", "")),
        )
    )

    selected: list[dict[str, Any]] = []
    per_file: dict[str, int] = {}
    for row in results:
        path = str(row["metadata"].get("path", "?"))
        if per_file.get(path, 0) >= 2:
            continue
        selected.append(row)
        per_file[path] = per_file.get(path, 0) + 1
        if len(selected) >= top_k:
            break
    return selected


def build_context(
    results: list[dict[str, Any]],
    max_context_chars: int,
) -> tuple[str, list[dict[str, Any]]]:
    blocks: list[str] = []
    used_results: list[dict[str, Any]] = []
    total_chars = 0

    for index, row in enumerate(results, start=1):
        metadata = row["metadata"]
        block = (
            f"[Source {index}]\n"
            f"path: {metadata.get('path', '?')}\n"
            f"kind: {metadata.get('kind', 'other')}\n"
            f"lines: {metadata.get('start_line', '?')}-{metadata.get('end_line', '?')}\n"
            f"score: {row['score']:.4f}\n\n"
            f"{row['document'].strip()}"
        )

        if blocks and total_chars + len(block) > max_context_chars:
            break

        blocks.append(block)
        used_results.append(row)
        total_chars += len(block)

    return "\n\n".join(blocks), used_results


def retrieve_rows(
    rows: list[dict[str, Any]],
    vectors: np.ndarray,
    query_vector: np.ndarray,
    fetch_k: int,
) -> list[dict[str, Any]]:
    if not rows or vectors.size == 0:
        return []

    query = query_vector.astype(np.float32, copy=False)
    if query.ndim != 1:
        query = query.reshape(-1)

    scores = vectors @ query
    fetch_count = min(fetch_k, len(rows))
    best_indices = np.argsort(scores)[::-1][:fetch_count]

    results: list[dict[str, Any]] = []
    for index in best_indices:
        row = rows[int(index)]
        results.append(
            {
                "document": str(row.get("document", "")),
                "metadata": row.get("metadata", {}) or {},
                "score": float(scores[int(index)]),
            }
        )
    return results


def ask_question(
    args: argparse.Namespace,
    rows: list[dict[str, Any]],
    vectors: np.ndarray,
    client,
    question: str,
) -> int:
    query_vector = np.asarray(
        client.embed_texts(args.embedding_model, [question])[0],
        dtype=np.float32,
    )
    retrieved = retrieve_rows(rows, vectors, query_vector, args.fetch_k)
    reranked = rerank_results(retrieved, args.top_k)
    if not reranked:
        safe_print("No context was retrieved from the local index.")
        safe_print("Try rebuilding the index with: python tools/rag/ingest.py --rebuild")
        return 1

    context, used_results = build_context(reranked, args.max_context_chars)

    if args.show_context:
        safe_print("")
        safe_print("Retrieved context")
        safe_print("-----------------")
        safe_print(context)
        safe_print("")

    user_prompt = (
        f"Question:\n{question}\n\n"
        f"Retrieved context:\n{context}\n\n"
        "Answer style:\n"
        "- Start with a direct answer.\n"
        "- Then list the strongest file-path evidence.\n"
        "- If something is uncertain, say that clearly.\n"
    )

    response = client.chat(
        model=args.model,
        system_prompt=SYSTEM_PROMPT,
        user_prompt=user_prompt,
        temperature=args.temperature,
        num_ctx=args.num_ctx,
        num_predict=args.num_predict,
    )

    content = response.get("message", {}).get("content", "").strip()
    if not content:
        safe_print("The model returned an empty answer.")
        return 1

    safe_print("")
    safe_print(content)
    safe_print("")
    safe_print("Sources")
    safe_print("-------")
    for row in used_results:
        safe_print(
            f"- {format_source_label(row['metadata'])} "
            f"(score={row['score']:.4f})"
        )
    return 0


def main() -> int:
    args = parse_args()
    ensure_rag_dirs()
    provider = normalize_provider(args.provider)
    args.model = args.model or default_chat_model_for_provider(provider)
    args.embedding_model = (
        args.embedding_model or default_embedding_model_for_provider(provider)
    )
    base_url = args.base_url or default_base_url_for_provider(provider)

    collection_name = collection_name_for_embedding_model(
        provider,
        args.embedding_model,
        prefix=args.collection_prefix,
    )
    manifest_path = manifest_path_for_embedding_model(
        provider,
        args.embedding_model,
        prefix=args.collection_prefix,
    )
    metadata_path = store_metadata_path_for_embedding_model(
        provider,
        args.embedding_model,
        prefix=args.collection_prefix,
    )
    vectors_path = store_vectors_path_for_embedding_model(
        provider,
        args.embedding_model,
        prefix=args.collection_prefix,
    )

    incompatibility_reason = detect_incompatible_chroma_collection(collection_name)
    if incompatibility_reason:
        if clear_legacy_chroma_cache():
            safe_print("Removed an incompatible legacy Chroma cache.")
        safe_print(f"Legacy cache reason: {incompatibility_reason}")
    else:
        clear_legacy_chroma_cache()

    if not manifest_path.exists():
        raise SystemExit(
            "No local RAG manifest was found.\n"
            "Run: python tools/rag/ingest.py"
        )

    rows, vectors = load_store(metadata_path, vectors_path)
    if not rows or vectors.size == 0:
        raise SystemExit(
            "The local vector store is empty.\n"
            "Run: python tools/rag/ingest.py --rebuild"
        )

    client = create_provider_client(
        provider,
        base_url=base_url,
        api_key=args.api_key,
    )
    if args.question:
        return ask_question(args, rows, vectors, client, " ".join(args.question).strip())

    safe_print(
        f"Zenith RAG shell using {provider_label(provider)} with model: {args.model}"
    )
    safe_print("Type your question. Use 'exit' or 'quit' to stop.")

    while True:
        try:
            question = input("\nzenith-rag> ").strip()
        except (EOFError, KeyboardInterrupt):
            safe_print("")
            return 0

        if not question:
            continue
        if question.lower() in {"exit", "quit"}:
            return 0

        ask_question(args, rows, vectors, client, question)


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except ProviderAPIError as exc:
        raise SystemExit(f"Provider error: {exc}") from exc
