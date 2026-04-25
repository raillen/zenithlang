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
        send(proc, {"jsonrpc": "2.0", "id": 1, "method": "initialize", "params": {}})
        init = expect(messages, errors, "initialize")
        assert init["result"]["capabilities"]["hoverProvider"] is True
        assert init["result"]["capabilities"]["definitionProvider"] is True
        assert init["result"]["capabilities"]["documentFormattingProvider"] is True
        assert "completionProvider" in init["result"]["capabilities"]

        dep_text = (
            "namespace app.util\n\n"
            "public func helper() -> int\n"
            "    return 42\n"
            "end\n\n"
            "func private_helper() -> int\n"
            "    return 0\n"
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
                "id": 5,
                "method": "textDocument/completion",
                "params": {"textDocument": {"uri": uri}, "position": {"line": 8, "character": 0}},
            },
        )
        completion = expect(messages, errors, "completion")
        labels = {item["label"] for item in completion["result"]}
        assert "func" in labels
        assert "result" in labels
        assert "add" in labels
        assert "util" in labels
        assert "helper" not in labels

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
        imported_member_labels = {item["label"] for item in imported_member_completion["result"]}
        assert "helper" in imported_member_labels
        assert "private_helper" not in imported_member_labels

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
                    "textDocument": {"uri": uri, "version": 4},
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
                    "textDocument": {"uri": uri, "version": 5},
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
                    "textDocument": {"uri": uri, "version": 6},
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
                    "textDocument": {"uri": uri, "version": 7},
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
                    "textDocument": {"uri": uri, "version": 8},
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

        send(proc, {"jsonrpc": "2.0", "id": 13, "method": "shutdown", "params": None})
        shutdown = expect(messages, errors, "shutdown")
        assert shutdown["result"] is None
        send(proc, {"jsonrpc": "2.0", "method": "exit", "params": None})
        proc.wait(timeout=5)
        print("lsp smoke ok")
        return 0
    finally:
        if proc.poll() is None:
            proc.kill()


if __name__ == "__main__":
    sys.exit(main())
