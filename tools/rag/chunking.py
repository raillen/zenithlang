from __future__ import annotations

import hashlib
import re
from dataclasses import dataclass
from pathlib import Path

from common import CODE_EXTENSIONS, DOC_EXTENSIONS, classify_path, relative_path


DOC_MAX_CHARS = 1800
DOC_MAX_LINES = 70
CODE_WINDOW_LINES = 60
CODE_OVERLAP_LINES = 12
HEADING_RE = re.compile(r"^\s{0,3}#{1,6}\s+(.*\S)\s*$")
ZENITH_SYMBOL_RE = re.compile(
    r"^\s*(namespace|func|type|enum|trait|impl)\s+([A-Za-z_][A-Za-z0-9_\.]*)"
)
C_SYMBOL_RE = re.compile(
    r"^\s*(?:static\s+)?(?:inline\s+)?[A-Za-z_][A-Za-z0-9_\s\*]*\s+([A-Za-z_][A-Za-z0-9_]*)\s*\("
)


@dataclass(frozen=True)
class Chunk:
    chunk_id: str
    text: str
    metadata: dict[str, object]


def build_chunks_for_path(path: Path, text: str) -> list[Chunk]:
    ext = path.suffix.lower()
    if ext in DOC_EXTENSIONS:
        return build_document_chunks(path, text)
    if ext in CODE_EXTENSIONS:
        return build_code_chunks(path, text)
    return []


def build_document_chunks(path: Path, text: str) -> list[Chunk]:
    rel_path = relative_path(path)
    kind, priority = classify_path(rel_path)
    lines = text.splitlines()
    chunks: list[Chunk] = []

    section_title = path.stem.replace("-", " ").replace("_", " ").strip() or rel_path
    section_start_line = 1
    section_lines: list[tuple[int, str]] = []

    def flush_section() -> None:
        nonlocal chunks
        if not section_lines:
            return
        chunks.extend(
            _build_document_section_chunks(
                rel_path=rel_path,
                kind=kind,
                priority=priority,
                title=section_title,
                section_lines=section_lines,
            )
        )

    for line_number, line in enumerate(lines, start=1):
        heading = HEADING_RE.match(line)
        if heading and section_lines:
            flush_section()
            section_title = heading.group(1).strip()
            section_start_line = line_number
            section_lines = [(line_number, line)]
            continue

        if not section_lines:
            section_start_line = line_number
        section_lines.append((line_number, line))

    flush_section()

    if not chunks and text.strip():
        chunks.append(
            _make_chunk(
                rel_path=rel_path,
                kind=kind,
                priority=priority,
                title=section_title,
                start_line=section_start_line,
                end_line=max(1, len(lines)),
                body=text.strip(),
            )
        )

    return chunks


def _build_document_section_chunks(
    *,
    rel_path: str,
    kind: str,
    priority: int,
    title: str,
    section_lines: list[tuple[int, str]],
) -> list[Chunk]:
    paragraphs: list[tuple[int, int, str]] = []
    current_lines: list[str] = []
    paragraph_start = section_lines[0][0]
    last_line_number = section_lines[0][0]

    def flush_paragraph() -> None:
        nonlocal current_lines
        if not current_lines:
            return
        body = "\n".join(current_lines).strip()
        if body:
            paragraphs.append((paragraph_start, last_line_number, body))
        current_lines = []

    for line_number, line in section_lines:
        if not current_lines:
            paragraph_start = line_number

        if line.strip():
            current_lines.append(line)
            last_line_number = line_number
        else:
            flush_paragraph()

    flush_paragraph()

    chunks: list[Chunk] = []
    buffer_texts: list[str] = []
    buffer_start = paragraphs[0][0] if paragraphs else section_lines[0][0]
    buffer_end = buffer_start

    def flush_buffer() -> None:
        nonlocal buffer_texts
        if not buffer_texts:
            return
        chunks.append(
            _make_chunk(
                rel_path=rel_path,
                kind=kind,
                priority=priority,
                title=title,
                start_line=buffer_start,
                end_line=buffer_end,
                body="\n\n".join(buffer_texts).strip(),
            )
        )
        buffer_texts = []

    for start_line, end_line, paragraph in paragraphs:
        candidate_parts = buffer_texts + [paragraph]
        candidate_text = "\n\n".join(candidate_parts)
        candidate_lines = end_line - buffer_start + 1
        if buffer_texts and (
            len(candidate_text) > DOC_MAX_CHARS or candidate_lines > DOC_MAX_LINES
        ):
            flush_buffer()
            buffer_start = start_line

        if not buffer_texts:
            buffer_start = start_line
        buffer_end = end_line
        buffer_texts.append(paragraph)

    flush_buffer()
    return chunks


def build_code_chunks(path: Path, text: str) -> list[Chunk]:
    rel_path = relative_path(path)
    kind, priority = classify_path(rel_path)
    raw_lines = text.splitlines()
    total_lines = len(raw_lines)
    if total_lines == 0:
        return []

    step = max(1, CODE_WINDOW_LINES - CODE_OVERLAP_LINES)
    chunks: list[Chunk] = []

    for start_index in range(0, total_lines, step):
        end_index = min(total_lines, start_index + CODE_WINDOW_LINES)
        slice_lines = raw_lines[start_index:end_index]
        if not any(line.strip() for line in slice_lines):
            continue

        trimmed_start = 0
        trimmed_end = len(slice_lines)

        while trimmed_start < trimmed_end and not slice_lines[trimmed_start].strip():
            trimmed_start += 1
        while trimmed_end > trimmed_start and not slice_lines[trimmed_end - 1].strip():
            trimmed_end -= 1

        body_lines = slice_lines[trimmed_start:trimmed_end]
        if not body_lines:
            continue

        start_line = start_index + trimmed_start + 1
        end_line = start_index + trimmed_end
        symbol = _nearest_symbol(raw_lines, start_line)
        title = symbol or path.stem
        body = "\n".join(body_lines).strip()
        chunks.append(
            _make_chunk(
                rel_path=rel_path,
                kind=kind,
                priority=priority,
                title=title,
                start_line=start_line,
                end_line=end_line,
                body=body,
            )
        )

        if end_index >= total_lines:
            break

    return chunks


def _nearest_symbol(lines: list[str], start_line: int) -> str | None:
    search_start = max(0, start_line - 40)
    search_end = min(len(lines), start_line + 1)

    for index in range(search_end - 1, search_start - 1, -1):
        line = lines[index]
        zenith_match = ZENITH_SYMBOL_RE.match(line)
        if zenith_match:
            return f"{zenith_match.group(1)} {zenith_match.group(2)}"

        c_match = C_SYMBOL_RE.match(line)
        if c_match:
            return f"function {c_match.group(1)}"

    return None


def _make_chunk(
    *,
    rel_path: str,
    kind: str,
    priority: int,
    title: str,
    start_line: int,
    end_line: int,
    body: str,
) -> Chunk:
    header_lines = [
        f"Path: {rel_path}",
        f"Kind: {kind}",
        f"Lines: {start_line}-{end_line}",
    ]
    cleaned_title = title.strip()
    if cleaned_title:
        header_lines.append(f"Section: {cleaned_title}")
    chunk_text = "\n".join(header_lines) + "\n\n" + body.strip()
    digest = hashlib.sha1(
        f"{rel_path}:{start_line}:{end_line}:{cleaned_title}".encode("utf-8")
    ).hexdigest()[:12]
    chunk_id = f"{rel_path}::{start_line}::{end_line}::{digest}"
    metadata = {
        "path": rel_path,
        "kind": kind,
        "priority": priority,
        "start_line": start_line,
        "end_line": end_line,
        "section": cleaned_title,
        "ext": Path(rel_path).suffix.lower(),
    }
    return Chunk(chunk_id=chunk_id, text=chunk_text, metadata=metadata)
