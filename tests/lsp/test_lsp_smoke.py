import json
import subprocess
import sys
import threading
import queue
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
EXE = ROOT / ("zt-lsp.exe" if sys.platform.startswith("win") else "zt-lsp")


def send(proc, obj):
    data = json.dumps(obj, separators=(",", ":")).encode("utf-8")
    proc.stdin.write(b"Content-Length: " + str(len(data)).encode("ascii") + b"\r\n\r\n" + data)
    proc.stdin.flush()


def start_reader(proc, messages, errors):
    def reader():
        try:
            while True:
                header = b""
                while b"\r\n\r\n" not in header:
                    ch = proc.stdout.read(1)
                    if not ch:
                        return
                    header += ch
                raw_header, rest = header.split(b"\r\n\r\n", 1)
                length = None
                for line in raw_header.decode("ascii", errors="replace").split("\r\n"):
                    if line.lower().startswith("content-length:"):
                        length = int(line.split(":", 1)[1].strip())
                if length is None:
                    errors.put("missing content-length")
                    return
                body = rest + proc.stdout.read(length - len(rest))
                messages.put(json.loads(body.decode("utf-8")))
        except Exception as exc:  # pragma: no cover - smoke harness failure path
            errors.put(repr(exc))

    thread = threading.Thread(target=reader, daemon=True)
    thread.start()
    return thread


def expect(messages, errors, label):
    try:
        return messages.get(timeout=5)
    except queue.Empty as exc:
        reader_error = None
        try:
            reader_error = errors.get_nowait()
        except queue.Empty:
            pass
        raise AssertionError(f"timeout waiting for {label}; reader_error={reader_error}") from exc


def run_stdlib_cwd_smoke():
    proc = subprocess.Popen(
        [str(EXE)],
        cwd=ROOT / "tools" / "vscode-zenith",
        stdin=subprocess.PIPE,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )
    messages = queue.Queue()
    errors = queue.Queue()
    start_reader(proc, messages, errors)
    try:
        send(
            proc,
            {
                "jsonrpc": "2.0",
                "id": 100,
                "method": "initialize",
                "params": {"locale": "pt-BR", "rootUri": ROOT.as_uri()},
            },
        )
        expect(messages, errors, "cwd initialize")
        uri = "file:///C:/tmp/cwd-main.zt"
        text = (
            "namespace app.main\n\n"
            "import std.io as io\n\n"
            "func main() -> int\n"
            "    io.\n"
            "    return 0\n"
            "end\n"
        )
        send(
            proc,
            {
                "jsonrpc": "2.0",
                "method": "textDocument/didOpen",
                "params": {
                    "textDocument": {
                        "uri": uri,
                        "languageId": "zenith",
                        "version": 1,
                        "text": text,
                    }
                },
            },
        )
        expect(messages, errors, "cwd diagnostics")
        send(
            proc,
            {
                "jsonrpc": "2.0",
                "id": 101,
                "method": "textDocument/completion",
                "params": {"textDocument": {"uri": uri}, "position": {"line": 5, "character": 7}},
            },
        )
        completion = expect(messages, errors, "cwd stdlib completion")
        labels = {item["label"] for item in completion["result"]}
        assert "print" in labels
        assert "read_line" in labels
        send(proc, {"jsonrpc": "2.0", "id": 102, "method": "shutdown", "params": None})
        expect(messages, errors, "cwd shutdown")
        send(proc, {"jsonrpc": "2.0", "method": "exit", "params": None})
        proc.wait(timeout=5)
    finally:
        if proc.poll() is None:
            proc.kill()


def run_english_zdoc_smoke():
    proc = subprocess.Popen(
        [str(EXE)],
        cwd=ROOT,
        stdin=subprocess.PIPE,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )
    messages = queue.Queue()
    errors = queue.Queue()
    start_reader(proc, messages, errors)
    try:
        send(
            proc,
            {
                "jsonrpc": "2.0",
                "id": 200,
                "method": "initialize",
                "params": {"locale": "en-US", "rootUri": ROOT.as_uri()},
            },
        )
        expect(messages, errors, "en initialize")
        uri = "file:///C:/tmp/en-main.zt"
        text = (
            "namespace app.main\n\n"
            "import std.text as text\n\n"
            "func main() -> int\n"
            "    text.\n"
            "    return 0\n"
            "end\n"
        )
        send(
            proc,
            {
                "jsonrpc": "2.0",
                "method": "textDocument/didOpen",
                "params": {
                    "textDocument": {
                        "uri": uri,
                        "languageId": "zenith",
                        "version": 1,
                        "text": text,
                    }
                },
            },
        )
        expect(messages, errors, "en diagnostics")
        send(
            proc,
            {
                "jsonrpc": "2.0",
                "id": 201,
                "method": "textDocument/completion",
                "params": {"textDocument": {"uri": uri}, "position": {"line": 5, "character": 9}},
            },
        )
        completion = expect(messages, errors, "en std.text completion")
        items = {item["label"]: item for item in completion["result"]}
        assert "trim" in items
        assert "Removes whitespace from both ends." in items["trim"]["documentation"]
        assert "**Documentation**" in items["trim"]["documentation"]
        assert "text.trim(value)" in items["trim"]["documentation"]
        send(proc, {"jsonrpc": "2.0", "id": 202, "method": "shutdown", "params": None})
        shutdown = expect(messages, errors, "en shutdown")
        assert shutdown["result"] is None
        send(proc, {"jsonrpc": "2.0", "method": "exit", "params": None})
        proc.wait(timeout=5)
    finally:
        if proc.poll() is None:
            proc.kill()


def main():
    build = subprocess.run([sys.executable, str(ROOT / "tools" / "build_lsp.py")], cwd=ROOT)
    if build.returncode != 0:
        return build.returncode

    proc = subprocess.Popen(
        [str(EXE)],
        cwd=ROOT,
        stdin=subprocess.PIPE,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )
    messages = queue.Queue()
    errors = queue.Queue()
    start_reader(proc, messages, errors)

    try:
        send(
            proc,
            {
                "jsonrpc": "2.0",
                "id": 1,
                "method": "initialize",
                "params": {"locale": "pt-BR", "rootUri": ROOT.as_uri()},
            },
        )
        init = expect(messages, errors, "initialize")
        assert init["result"]["capabilities"]["hoverProvider"] is True
        assert init["result"]["capabilities"]["definitionProvider"] is True
        assert init["result"]["capabilities"]["referencesProvider"] is True
        assert init["result"]["capabilities"]["renameProvider"]["prepareProvider"] is True
        assert "signatureHelpProvider" in init["result"]["capabilities"]
        assert init["result"]["capabilities"]["documentFormattingProvider"] is True
        assert init["result"]["capabilities"]["documentSymbolProvider"] is True
        assert init["result"]["capabilities"]["workspaceSymbolProvider"] is True
        assert "semanticTokensProvider" in init["result"]["capabilities"]
        assert "completionProvider" in init["result"]["capabilities"]

        dep_text = (
            "namespace app.util\n\n"
            "public func helper() -> int\n"
            "    return 42\n"
            "end\n\n"
            "func private_helper() -> int\n"
            "    return 0\n"
            "end\n\n"
            "public struct HelperBox\n"
            "    value: int\n"
            "    label: text\n"
            "end\n\n"
            "apply HelperBox\n"
            "    -- Descreve a caixa para exibicao.\n"
            "    public func describe() -> text\n"
            "        return \"box\"\n"
            "    end\n\n"
            "    func hidden() -> int\n"
            "        return 0\n"
            "    end\n"
            "end\n"
        )
        dep_uri = "file:///C:/tmp/util.zt"
        send(
            proc,
            {
                "jsonrpc": "2.0",
                "method": "textDocument/didOpen",
                "params": {
                    "textDocument": {
                        "uri": dep_uri,
                        "languageId": "zenith",
                        "version": 1,
                        "text": dep_text,
                    }
                },
            },
        )
        dep_diag = expect(messages, errors, "dep diagnostics")
        assert dep_diag["method"] == "textDocument/publishDiagnostics"
        assert dep_diag["params"]["uri"] == dep_uri

        text = (
            "namespace app.main\n\n"
            "import app.util as util\n\n"
            "func add(a: int, b: int) -> int\n"
            "    return a + b\n"
            "end\n\n"
            "func main() -> int\n"
            "    return add(1)\n"
            "end\n"
        )
        uri = "file:///C:/tmp/main.zt"
        send(
            proc,
            {
                "jsonrpc": "2.0",
                "method": "textDocument/didOpen",
                "params": {
                    "textDocument": {
                        "uri": uri,
                        "languageId": "zenith",
                        "version": 1,
                        "text": text,
                    }
                },
            },
        )
        diag = expect(messages, errors, "publishDiagnostics")
        assert diag["method"] == "textDocument/publishDiagnostics"
        assert diag["params"]["uri"] == uri
        assert any("missing argument" in item["message"] for item in diag["params"]["diagnostics"])

        send(
            proc,
            {
                "jsonrpc": "2.0",
                "id": 2,
                "method": "textDocument/hover",
                "params": {"textDocument": {"uri": uri}, "position": {"line": 4, "character": 6}},
            },
        )
        hover = expect(messages, errors, "hover")
        assert "func add" in hover["result"]["contents"]["value"]

        send(
            proc,
            {
                "jsonrpc": "2.0",
                "id": 3,
                "method": "textDocument/definition",
                "params": {"textDocument": {"uri": uri}, "position": {"line": 9, "character": 13}},
            },
        )
        definition = expect(messages, errors, "definition")
        assert definition["result"]["range"]["start"]["line"] == 4

        send(
            proc,
            {
                "jsonrpc": "2.0",
                "id": 62,
                "method": "textDocument/signatureHelp",
                "params": {"textDocument": {"uri": uri}, "position": {"line": 9, "character": 16}},
            },
        )
        signature_help = expect(messages, errors, "signature help")
        assert signature_help["result"]["signatures"][0]["label"] == "func add(a: int, b: int) -> int"
        assert signature_help["result"]["signatures"][0]["parameters"][0]["label"] == "a: int"
        assert "documentation" in signature_help["result"]["signatures"][0]
        assert "Assinatura" in signature_help["result"]["signatures"][0]["documentation"]
        assert signature_help["result"]["activeParameter"] == 0

        send(
            proc,
            {
                "jsonrpc": "2.0",
                "id": 4,
                "method": "textDocument/formatting",
                "params": {"textDocument": {"uri": uri}, "options": {"tabSize": 4, "insertSpaces": True}},
            },
        )
        formatting = expect(messages, errors, "formatting")
        assert isinstance(formatting["result"], list)

        send(
            proc,
            {
                "jsonrpc": "2.0",
                "id": 66,
                "method": "textDocument/semanticTokens/full",
                "params": {"textDocument": {"uri": uri}},
            },
        )
        semantic_tokens = expect(messages, errors, "semantic tokens")
        assert isinstance(semantic_tokens["result"]["data"], list)
        assert len(semantic_tokens["result"]["data"]) > 0
        assert len(semantic_tokens["result"]["data"]) % 5 == 0

        send(
            proc,
            {
                "jsonrpc": "2.0",
                "id": 5,
                "method": "textDocument/completion",
                "params": {"textDocument": {"uri": uri}, "position": {"line": 8, "character": 0}},
            },
        )
        completion = expect(messages, errors, "completion")
        items_by_label = {item["label"]: item for item in completion["result"]}
        labels = set(items_by_label)
        assert "func" in labels
        assert "result" in labels
        assert "any" in labels
        assert "set" in labels
        assert "using" in labels
        assert "capture" in labels
        assert "apply" in labels
        assert "repeat" in labels
        assert "some" in labels
        assert "add" in labels
        assert "util" in labels
        assert "helper" not in labels
        assert "fmt" in labels
        assert "int()" in labels
        assert "float()" in labels
        assert "len()" in labels
        assert "to_text()" in labels
        assert items_by_label["fmt"]["insertText"] == 'fmt "${1:texto {valor}}"'
        assert items_by_label["int()"]["insertText"] == "int(${1:value})"
        assert items_by_label["to_text()"]["detail"] == "to_text(value) -> text"
        assert "TextRepresentable<T>" in items_by_label["to_text()"]["documentation"]
        assert "exibicao, logs e mensagens" in items_by_label["to_text()"]["documentation"]
        assert "Assinatura" in items_by_label["fmt"]["documentation"]

        send(
            proc,
            {
                "jsonrpc": "2.0",
                "id": 50,
                "method": "textDocument/completion",
                "params": {"textDocument": {"uri": uri}, "position": {"line": 2, "character": 11}},
            },
        )
        import_completion = expect(messages, errors, "import path completion")
        import_labels = {item["label"] for item in import_completion["result"]}
        assert "std.io" in import_labels
        assert "app.util" in import_labels
        assert "func" not in import_labels

        cross_file_text = (
            "namespace app.main\n\n"
            "import app.util as util\n\n"
            "func main() -> int\n"
            "    return util.helper()\n"
            "end\n"
        )
        send(
            proc,
            {
                "jsonrpc": "2.0",
                "method": "textDocument/didChange",
                "params": {
                    "textDocument": {"uri": uri, "version": 2},
                    "contentChanges": [{"text": cross_file_text}],
                },
            },
        )
        expect(messages, errors, "cross-file diagnostics")
        send(
            proc,
            {
                "jsonrpc": "2.0",
                "id": 6,
                "method": "textDocument/definition",
                "params": {"textDocument": {"uri": uri}, "position": {"line": 5, "character": 19}},
            },
        )
        cross_definition = expect(messages, errors, "cross-file definition")
        assert cross_definition["result"]["uri"] == dep_uri
        assert cross_definition["result"]["range"]["start"]["line"] == 2

        send(
            proc,
            {
                "jsonrpc": "2.0",
                "id": 60,
                "method": "textDocument/references",
                "params": {
                    "textDocument": {"uri": uri},
                    "position": {"line": 5, "character": 19},
                    "context": {"includeDeclaration": True},
                },
            },
        )
        references = expect(messages, errors, "cross-file references")
        reference_locations = {(item["uri"], item["range"]["start"]["line"]) for item in references["result"]}
        assert (dep_uri, 2) in reference_locations
        assert (uri, 5) in reference_locations

        send(
            proc,
            {
                "jsonrpc": "2.0",
                "id": 61,
                "method": "textDocument/references",
                "params": {
                    "textDocument": {"uri": uri},
                    "position": {"line": 5, "character": 19},
                    "context": {"includeDeclaration": False},
                },
            },
        )
        references_without_decl = expect(messages, errors, "references without declaration")
        reference_without_decl_locations = {
            (item["uri"], item["range"]["start"]["line"]) for item in references_without_decl["result"]
        }
        assert (dep_uri, 2) not in reference_without_decl_locations
        assert (uri, 5) in reference_without_decl_locations

        send(
            proc,
            {
                "jsonrpc": "2.0",
                "id": 64,
                "method": "textDocument/prepareRename",
                "params": {
                    "textDocument": {"uri": uri},
                    "position": {"line": 5, "character": 19},
                },
            },
        )
        prepare_rename = expect(messages, errors, "prepare rename")
        assert prepare_rename["result"]["placeholder"] == "helper"
        assert prepare_rename["result"]["range"]["start"]["line"] == 5
        assert prepare_rename["result"]["range"]["start"]["character"] == 16

        send(
            proc,
            {
                "jsonrpc": "2.0",
                "id": 65,
                "method": "textDocument/prepareRename",
                "params": {
                    "textDocument": {"uri": uri},
                    "position": {"line": 5, "character": 12},
                },
            },
        )
        prepare_alias_rename = expect(messages, errors, "prepare alias rename")
        assert prepare_alias_rename["result"] is None

        send(
            proc,
            {
                "jsonrpc": "2.0",
                "id": 63,
                "method": "textDocument/rename",
                "params": {
                    "textDocument": {"uri": uri},
                    "position": {"line": 5, "character": 19},
                    "newName": "assist",
                },
            },
        )
        rename = expect(messages, errors, "rename")
        assert dep_uri in rename["result"]["changes"]
        assert uri in rename["result"]["changes"]
        dep_edits = rename["result"]["changes"][dep_uri]
        main_edits = rename["result"]["changes"][uri]
        assert any(
            edit["range"]["start"]["line"] == 2
            and edit["range"]["start"]["character"] == 12
            and edit["newText"] == "assist"
            for edit in dep_edits
        )
        assert any(
            edit["range"]["start"]["line"] == 5
            and edit["range"]["start"]["character"] == 16
            and edit["newText"] == "assist"
            for edit in main_edits
        )

        imported_member_text = (
            "namespace app.main\n\n"
            "import app.util as util\n\n"
            "func main() -> int\n"
            "    return util.\n"
            "end\n"
        )
        send(
            proc,
            {
                "jsonrpc": "2.0",
                "method": "textDocument/didChange",
                "params": {
                    "textDocument": {"uri": uri, "version": 3},
                    "contentChanges": [{"text": imported_member_text}],
                },
            },
        )
        expect(messages, errors, "imported member diagnostics")
        send(
            proc,
            {
                "jsonrpc": "2.0",
                "id": 7,
                "method": "textDocument/completion",
                "params": {"textDocument": {"uri": uri}, "position": {"line": 5, "character": 16}},
            },
        )
        imported_member_completion = expect(messages, errors, "imported member completion")
        imported_member_items = {item["label"]: item for item in imported_member_completion["result"]}
        imported_member_labels = set(imported_member_items)
        assert "helper" in imported_member_labels
        assert "private_helper" not in imported_member_labels
        assert imported_member_items["helper"]["detail"] == "app.util.helper() -> int"
        assert "Sem documentacao local" in imported_member_items["helper"]["documentation"]

        stdlib_member_text = (
            "namespace app.main\n\n"
            "import std.io as io\n\n"
            "func main() -> int\n"
            "    io.\n"
            "    return 0\n"
            "end\n"
        )
        send(
            proc,
            {
                "jsonrpc": "2.0",
                "method": "textDocument/didChange",
                "params": {
                    "textDocument": {"uri": uri, "version": 4},
                    "contentChanges": [{"text": stdlib_member_text}],
                },
            },
        )
        expect(messages, errors, "stdlib member diagnostics")
        send(
            proc,
            {
                "jsonrpc": "2.0",
                "id": 70,
                "method": "textDocument/completion",
                "params": {"textDocument": {"uri": uri}, "position": {"line": 5, "character": 7}},
            },
        )
        stdlib_member_completion = expect(messages, errors, "stdlib member completion")
        stdlib_member_items = {item["label"]: item for item in stdlib_member_completion["result"]}
        stdlib_member_labels = set(stdlib_member_items)
        assert "print" in stdlib_member_labels
        assert "read_line" in stdlib_member_labels
        assert "read_all" in stdlib_member_labels
        assert "input" in stdlib_member_labels
        assert "zt_host_write_stdout" not in stdlib_member_labels
        assert "@param value" in stdlib_member_items["print"]["documentation"]
        assert "write" in stdlib_member_items["print"]["documentation"]
        assert "io.print(value)" in stdlib_member_items["print"]["documentation"]

        stdlib_shortcut_text = (
            "namespace app.main\n\n"
            "import std.io as io\n\n"
            "func main() -> int\n"
            "    pri\n"
            "    return 0\n"
            "end\n"
        )
        send(
            proc,
            {
                "jsonrpc": "2.0",
                "method": "textDocument/didChange",
                "params": {
                    "textDocument": {"uri": uri, "version": 5},
                    "contentChanges": [{"text": stdlib_shortcut_text}],
                },
            },
        )
        expect(messages, errors, "stdlib shortcut diagnostics")
        send(
            proc,
            {
                "jsonrpc": "2.0",
                "id": 71,
                "method": "textDocument/completion",
                "params": {"textDocument": {"uri": uri}, "position": {"line": 5, "character": 7}},
            },
        )
        stdlib_shortcut_completion = expect(messages, errors, "stdlib shortcut completion")
        stdlib_items_by_label = {item["label"]: item for item in stdlib_shortcut_completion["result"]}
        assert "print()" in stdlib_items_by_label
        assert "read_line()" in stdlib_items_by_label
        assert stdlib_items_by_label["print()"]["insertText"] == "io.print(${1:value})"
        assert "std.io.print" in stdlib_items_by_label["print()"]["detail"]
        assert "@param value" in stdlib_items_by_label["print()"]["documentation"]

        member_text = (
            "namespace app.main\n\n"
            "func teste() -> void\n"
            "    var a: int = 10\n"
            "    a.\n"
            "end\n"
        )
        send(
            proc,
            {
                "jsonrpc": "2.0",
                "method": "textDocument/didChange",
                "params": {
                    "textDocument": {"uri": uri, "version": 6},
                    "contentChanges": [{"text": member_text}],
                },
            },
        )
        expect(messages, errors, "member diagnostics")
        send(
            proc,
            {
                "jsonrpc": "2.0",
                "id": 8,
                "method": "textDocument/completion",
                "params": {"textDocument": {"uri": uri}, "position": {"line": 4, "character": 6}},
            },
        )
        member_completion = expect(messages, errors, "member completion")
        assert member_completion["result"] == []

        list_member_text = (
            "namespace app.main\n\n"
            "func teste() -> void\n"
            "    var values: list<int> = [1, 2]\n"
            "    values.\n"
            "end\n"
        )
        send(
            proc,
            {
                "jsonrpc": "2.0",
                "method": "textDocument/didChange",
                "params": {
                    "textDocument": {"uri": uri, "version": 7},
                    "contentChanges": [{"text": list_member_text}],
                },
            },
        )
        expect(messages, errors, "list member diagnostics")
        send(
            proc,
            {
                "jsonrpc": "2.0",
                "id": 9,
                "method": "textDocument/completion",
                "params": {"textDocument": {"uri": uri}, "position": {"line": 4, "character": 11}},
            },
        )
        list_member_completion = expect(messages, errors, "list member completion")
        list_member_labels = {item["label"] for item in list_member_completion["result"]}
        assert "get" in list_member_labels
        assert "func" not in list_member_labels

        list_partial_text = (
            "namespace app.main\n\n"
            "func teste() -> void\n"
            "    var values: list<int> = [1, 2]\n"
            "    values.g\n"
            "end\n"
        )
        send(
            proc,
            {
                "jsonrpc": "2.0",
                "method": "textDocument/didChange",
                "params": {
                    "textDocument": {"uri": uri, "version": 8},
                    "contentChanges": [{"text": list_partial_text}],
                },
            },
        )
        expect(messages, errors, "list partial member diagnostics")
        send(
            proc,
            {
                "jsonrpc": "2.0",
                "id": 10,
                "method": "textDocument/completion",
                "params": {"textDocument": {"uri": uri}, "position": {"line": 4, "character": 12}},
            },
        )
        list_partial_completion = expect(messages, errors, "list partial member completion")
        list_partial_labels = {item["label"] for item in list_partial_completion["result"]}
        assert "get" in list_partial_labels
        assert "func" not in list_partial_labels

        map_member_text = (
            "namespace app.main\n\n"
            "func teste() -> void\n"
            "    var scores: map<text, int> = {}\n"
            "    scores.\n"
            "end\n"
        )
        send(
            proc,
            {
                "jsonrpc": "2.0",
                "method": "textDocument/didChange",
                "params": {
                    "textDocument": {"uri": uri, "version": 9},
                    "contentChanges": [{"text": map_member_text}],
                },
            },
        )
        expect(messages, errors, "map member diagnostics")
        send(
            proc,
            {
                "jsonrpc": "2.0",
                "id": 11,
                "method": "textDocument/completion",
                "params": {"textDocument": {"uri": uri}, "position": {"line": 4, "character": 11}},
            },
        )
        map_member_completion = expect(messages, errors, "map member completion")
        map_member_labels = {item["label"] for item in map_member_completion["result"]}
        assert "get" in map_member_labels
        assert "func" not in map_member_labels

        struct_member_text = (
            "namespace app.main\n\n"
            "struct Player\n"
            "    name: text\n"
            "    hp: int\n"
            "end\n\n"
            "func teste() -> void\n"
            "    var player: Player = Player(name: \"Ada\", hp: 10)\n"
            "    player.\n"
            "end\n"
        )
        send(
            proc,
            {
                "jsonrpc": "2.0",
                "method": "textDocument/didChange",
                "params": {
                    "textDocument": {"uri": uri, "version": 10},
                    "contentChanges": [{"text": struct_member_text}],
                },
            },
        )
        expect(messages, errors, "struct member diagnostics")
        send(
            proc,
            {
                "jsonrpc": "2.0",
                "id": 12,
                "method": "textDocument/completion",
                "params": {"textDocument": {"uri": uri}, "position": {"line": 9, "character": 11}},
            },
        )
        struct_member_completion = expect(messages, errors, "struct member completion")
        struct_member_labels = {item["label"] for item in struct_member_completion["result"]}
        assert "name" in struct_member_labels
        assert "hp" in struct_member_labels
        assert "func" not in struct_member_labels

        imported_struct_member_text = (
            "namespace app.main\n\n"
            "import app.util as util\n\n"
            "func teste() -> void\n"
            "    var box: util.HelperBox = util.HelperBox(value: 1, label: \"Ada\")\n"
            "    box.\n"
            "end\n"
        )
        send(
            proc,
            {
                "jsonrpc": "2.0",
                "method": "textDocument/didChange",
                "params": {
                    "textDocument": {"uri": uri, "version": 11},
                    "contentChanges": [{"text": imported_struct_member_text}],
                },
            },
        )
        expect(messages, errors, "imported struct member diagnostics")
        send(
            proc,
            {
                "jsonrpc": "2.0",
                "id": 72,
                "method": "textDocument/completion",
                "params": {"textDocument": {"uri": uri}, "position": {"line": 6, "character": 8}},
            },
        )
        imported_struct_completion = expect(messages, errors, "imported struct member completion")
        imported_struct_labels = {item["label"] for item in imported_struct_completion["result"]}
        assert "value" in imported_struct_labels
        assert "label" in imported_struct_labels
        assert "describe" in imported_struct_labels
        assert "hidden" not in imported_struct_labels
        describe_item = next(item for item in imported_struct_completion["result"] if item["label"] == "describe")
        assert "describe() -> text" in describe_item["detail"]
        assert "Descreve a caixa" in describe_item["documentation"]

        imported_method_signature_text = (
            "namespace app.main\n\n"
            "import app.util as util\n\n"
            "func teste() -> void\n"
            "    var box: util.HelperBox = util.HelperBox(value: 1, label: \"Ada\")\n"
            "    box.describe(\n"
            "end\n"
        )
        send(
            proc,
            {
                "jsonrpc": "2.0",
                "method": "textDocument/didChange",
                "params": {
                    "textDocument": {"uri": uri, "version": 12},
                    "contentChanges": [{"text": imported_method_signature_text}],
                },
            },
        )
        expect(messages, errors, "imported method signature diagnostics")
        send(
            proc,
            {
                "jsonrpc": "2.0",
                "id": 73,
                "method": "textDocument/signatureHelp",
                "params": {"textDocument": {"uri": uri}, "position": {"line": 6, "character": 17}},
            },
        )
        imported_method_signature = expect(messages, errors, "imported method signature")
        assert imported_method_signature["result"]["signatures"][0]["label"] == "func describe() -> text"
        assert "Descreve a caixa" in imported_method_signature["result"]["signatures"][0]["documentation"]

        send(
            proc,
            {
                "jsonrpc": "2.0",
                "method": "textDocument/didChange",
                "params": {
                    "textDocument": {"uri": uri, "version": 13},
                    "contentChanges": [{"text": struct_member_text}],
                },
            },
        )
        expect(messages, errors, "struct member restore diagnostics")

        send(
            proc,
            {
                "jsonrpc": "2.0",
                "id": 13,
                "method": "textDocument/documentSymbol",
                "params": {"textDocument": {"uri": uri}},
            },
        )
        document_symbols = expect(messages, errors, "document symbols")
        symbol_names = {item["name"] for item in document_symbols["result"]}
        assert "Player" in symbol_names
        assert "teste" in symbol_names
        player_symbol = next(item for item in document_symbols["result"] if item["name"] == "Player")
        player_children = {item["name"] for item in player_symbol.get("children", [])}
        assert "name" in player_children
        assert "hp" in player_children

        send(
            proc,
            {
                "jsonrpc": "2.0",
                "id": 14,
                "method": "workspace/symbol",
                "params": {"query": "helper"},
            },
        )
        workspace_symbols = expect(messages, errors, "workspace symbols")
        workspace_names = {item["name"] for item in workspace_symbols["result"]}
        assert "helper" in workspace_names
        assert any(item["containerName"] == "app.util" for item in workspace_symbols["result"] if item["name"] == "helper")

        send(proc, {"jsonrpc": "2.0", "id": 15, "method": "shutdown", "params": None})
        shutdown = expect(messages, errors, "shutdown")
        assert shutdown["result"] is None
        send(proc, {"jsonrpc": "2.0", "method": "exit", "params": None})
        proc.wait(timeout=5)
        run_stdlib_cwd_smoke()
        run_english_zdoc_smoke()
        print("lsp smoke ok")
        return 0
    finally:
        if proc.poll() is None:
            proc.kill()


if __name__ == "__main__":
    sys.exit(main())
