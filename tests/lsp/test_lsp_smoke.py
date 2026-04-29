import json
import subprocess
import sys
import threading
import queue
import time
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
EXE = ROOT / ("zt-lsp.exe" if sys.platform.startswith("win") else "zt-lsp")


def send(proc, obj):
    data = json.dumps(obj, separators=(",", ":")).encode("utf-8")
    proc.stdin.write(b"Content-Length: " + str(len(data)).encode("ascii") + b"\r\n\r\n" + data)
    proc.stdin.flush()


def send_raw_json(proc, raw):
    data = raw.encode("utf-8")
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


def decode_semantic_tokens(text, data):
    token_types = [
        "namespace",
        "type",
        "function",
        "variable",
        "property",
        "keyword",
        "modifier",
        "string",
        "number",
    ]
    token_modifiers = ["declaration", "readonly", "public"]
    lines = text.splitlines()
    result = []
    line = 0
    start = 0
    for index in range(0, len(data), 5):
        delta_line, delta_start, length, token_type, modifiers = data[index : index + 5]
        line += delta_line
        start = start + delta_start if delta_line == 0 else delta_start
        lexeme = lines[line][start : start + length]
        result.append(
            {
                "text": lexeme,
                "type": token_types[token_type],
                "modifiers": {
                    token_modifiers[bit]
                    for bit in range(len(token_modifiers))
                    if modifiers & (1 << bit)
                },
            }
        )
    return result


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

        builtin_text = (
            "namespace app.main\n\n"
            "import std.io as io\n\n"
            "func main()\n"
            "    \n"
            "    print(\"Hello from Zenith\")\n"
            "end\n"
        )
        send(
            proc,
            {
                "jsonrpc": "2.0",
                "method": "textDocument/didChange",
                "params": {
                    "textDocument": {"uri": uri, "version": 2},
                    "contentChanges": [{"text": builtin_text}],
                },
            },
        )
        expect(messages, errors, "cwd builtin diagnostics")
        send(
            proc,
            {
                "jsonrpc": "2.0",
                "id": 103,
                "method": "textDocument/completion",
                "params": {"textDocument": {"uri": uri}, "position": {"line": 5, "character": 4}},
            },
        )
        builtin_completion = expect(messages, errors, "cwd builtin completion")
        print_items = [item for item in builtin_completion["result"] if item["label"] == "print()"]
        assert any(item.get("detail") == "print(value) -> void" for item in print_items)
        assert not any(str(item.get("insertText", "")).startswith("io.print") for item in print_items)
        send(
            proc,
            {
                "jsonrpc": "2.0",
                "id": 104,
                "method": "textDocument/hover",
                "params": {"textDocument": {"uri": uri}, "position": {"line": 6, "character": 6}},
            },
        )
        builtin_hover = expect(messages, errors, "cwd builtin hover")
        assert "Origem: builtin." in builtin_hover["result"]["contents"]["value"]
        send(
            proc,
            {
                "jsonrpc": "2.0",
                "id": 105,
                "method": "textDocument/signatureHelp",
                "params": {"textDocument": {"uri": uri}, "position": {"line": 6, "character": 10}},
            },
        )
        builtin_signature = expect(messages, errors, "cwd builtin signature")
        assert builtin_signature["result"]["signatures"][0]["label"] == "print(value) -> void"
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


def run_lsp_robustness_smoke():
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
        send(proc, {"jsonrpc": "2.0", "id": 300, "method": "initialize", "params": {"locale": "pt-BR", "rootUri": ROOT.as_uri()}})
        expect(messages, errors, "robust initialize")

        started = time.monotonic()
        for index in range(30):
            uri = f"file:///C:/tmp/robust-{index}.zt"
            funcs = "\n".join(
                f"public func value_{index}_{item}() -> int\n    return {item}\nend\n"
                for item in range(12)
            )
            text = f"namespace stress.mod{index}\n\n{funcs}"
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
            diag = expect(messages, errors, f"robust diagnostics {index}")
            assert diag["params"]["version"] == 1
        assert time.monotonic() - started < 20

        uri = "file:///C:/tmp/robust-main.zt"
        text = (
            "namespace stress.main\n\n"
            "import stress.mod10 as m10\n\n"
            "func main() -> int\n"
            "    return m10.value_10_1()\n"
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
        expect(messages, errors, "robust main diagnostics")

        before = time.monotonic()
        send(proc, {"jsonrpc": "2.0", "id": 301, "method": "textDocument/completion", "params": {"textDocument": {"uri": uri}, "position": {"line": 5, "character": 15}}})
        completion = expect(messages, errors, "robust completion")
        assert time.monotonic() - before < 5
        assert any(item["label"] == "value_10_1" for item in completion["result"])

        before = time.monotonic()
        send(proc, {"jsonrpc": "2.0", "id": 302, "method": "textDocument/hover", "params": {"textDocument": {"uri": uri}, "position": {"line": 5, "character": 15}}})
        hover = expect(messages, errors, "robust hover")
        assert time.monotonic() - before < 5
        assert hover["result"] is not None

        changing_bad = "namespace stress.main\n\nfunc main() -> int\n    return missing()\nend\n"
        changing_good = "namespace stress.main\n\nfunc main() -> int\n    return 0\nend\n"
        for version, body in [(2, changing_bad), (3, changing_good)]:
            send(
                proc,
                {
                    "jsonrpc": "2.0",
                    "method": "textDocument/didChange",
                    "params": {
                        "textDocument": {"uri": uri, "version": version},
                        "contentChanges": [{"text": body}],
                    },
                },
            )
        first_diag = expect(messages, errors, "rapid diagnostics first")
        second_diag = expect(messages, errors, "rapid diagnostics second")
        assert {first_diag["params"]["version"], second_diag["params"]["version"]} == {2, 3}
        latest_diag = first_diag if first_diag["params"]["version"] == 3 else second_diag
        assert latest_diag["params"]["diagnostics"] == []

        disappearing_uri = "file:///C:/tmp/disappearing.zt"
        disappearing_text = (
            "namespace stress.gone\n\n"
            "public func gone_symbol() -> int\n"
            "    return 1\n"
            "end\n"
        )
        send(
            proc,
            {
                "jsonrpc": "2.0",
                "method": "textDocument/didOpen",
                "params": {
                    "textDocument": {
                        "uri": disappearing_uri,
                        "languageId": "zenith",
                        "version": 1,
                        "text": disappearing_text,
                    }
                },
            },
        )
        expect(messages, errors, "disappearing diagnostics")
        send(proc, {"jsonrpc": "2.0", "id": 303, "method": "workspace/symbol", "params": {"query": "gone_symbol"}})
        before_close_symbols = expect(messages, errors, "symbols before close")
        assert any(item["name"] == "gone_symbol" for item in before_close_symbols["result"])
        send(proc, {"jsonrpc": "2.0", "method": "textDocument/didClose", "params": {"textDocument": {"uri": disappearing_uri}}})
        expect(messages, errors, "disappearing close diagnostics")
        send(proc, {"jsonrpc": "2.0", "id": 304, "method": "workspace/symbol", "params": {"query": "gone_symbol"}})
        after_close_symbols = expect(messages, errors, "symbols after close")
        assert not any(item["name"] == "gone_symbol" for item in after_close_symbols["result"])

        send_raw_json(proc, '{"jsonrpc":"2.0","id":399,"method":')
        send(proc, {"jsonrpc": "2.0", "id": 305, "method": "workspace/symbol", "params": {"query": "value_10_1"}})
        still_alive = expect(messages, errors, "post-invalid-json workspace symbol")
        assert any(item["name"] == "value_10_1" for item in still_alive["result"])

        send(proc, {"jsonrpc": "2.0", "id": 306, "method": "shutdown", "params": None})
        shutdown = expect(messages, errors, "robust shutdown")
        assert shutdown["result"] is None
        send(proc, {"jsonrpc": "2.0", "method": "exit", "params": None})
        assert proc.wait(timeout=5) == 0
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
            "public trait Displayable\n"
            "    func label() -> text\n"
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

        distant_text = (
            "namespace lib.extra\n\n"
            "public func distant() -> int\n"
            "    return 7\n"
            "end\n"
        )
        distant_uri = "file:///C:/tmp/extra.zt"
        send(
            proc,
            {
                "jsonrpc": "2.0",
                "method": "textDocument/didOpen",
                "params": {
                    "textDocument": {
                        "uri": distant_uri,
                        "languageId": "zenith",
                        "version": 1,
                        "text": distant_text,
                    }
                },
            },
        )
        distant_diag = expect(messages, errors, "distant diagnostics")
        assert distant_diag["method"] == "textDocument/publishDiagnostics"
        assert distant_diag["params"]["uri"] == distant_uri

        text = (
            "namespace app.main\n\n"
            "import app.util as util\n\n"
            "func add(a: int, b: int) -> int\n"
            "    return a + b\n"
            "end\n\n"
            "func main() -> int\n"
            "    return add(1)\n"
            "end\n"
            "\n"
            "func named() -> int\n"
            "    return add(a: 1, b: 2)\n"
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
        diag_keys = {
            (
                item.get("code"),
                item["range"]["start"]["line"],
                item["range"]["start"]["character"],
                item["range"]["end"]["line"],
                item["range"]["end"]["character"],
                item["message"],
            )
            for item in diag["params"]["diagnostics"]
        }
        assert len(diag_keys) == len(diag["params"]["diagnostics"])

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
        assert "Origem: local." in hover["result"]["contents"]["value"]

        send(
            proc,
            {
                "jsonrpc": "2.0",
                "id": 21,
                "method": "textDocument/hover",
                "params": {"textDocument": {"uri": uri}, "position": {"line": 5, "character": 11}},
            },
        )
        param_hover = expect(messages, errors, "param hover")
        assert "a: int" in param_hover["result"]["contents"]["value"]
        assert "Origem: local." in param_hover["result"]["contents"]["value"]

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
                "id": 22,
                "method": "textDocument/definition",
                "params": {"textDocument": {"uri": uri}, "position": {"line": 5, "character": 11}},
            },
        )
        param_definition = expect(messages, errors, "param definition")
        assert param_definition["result"]["range"]["start"]["line"] == 4

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
                "id": 63,
                "method": "textDocument/signatureHelp",
                "params": {"textDocument": {"uri": uri}, "position": {"line": 13, "character": 24}},
            },
        )
        named_signature_help = expect(messages, errors, "named argument signature help")
        assert named_signature_help["result"]["signatures"][0]["label"] == "func add(a: int, b: int) -> int"
        assert named_signature_help["result"]["activeParameter"] == 1

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
        decoded_tokens = decode_semantic_tokens(text, semantic_tokens["result"]["data"])
        namespace_tokens = [(item["text"], item["type"]) for item in decoded_tokens if item["text"] in {"app", "main"}]
        assert ("app", "namespace") in namespace_tokens
        assert ("main", "namespace") in namespace_tokens
        assert any(
            item["text"] == "add" and item["type"] == "function" and "declaration" in item["modifiers"]
            for item in decoded_tokens
        )
        assert any(item["text"] == "int" and item["type"] == "type" for item in decoded_tokens)
        assert any(item["text"] == "a" and item["type"] == "variable" for item in decoded_tokens)

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
        assert "func main" in labels
        assert "result" in labels
        assert "any" in labels
        assert "set" in labels
        assert "using" in labels
        assert "capture" in labels
        assert "apply" in labels
        assert "apply Type" in labels
        assert "apply Trait to Type" in labels
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
        assert items_by_label["func main"]["insertText"] == "func main()\n    ${0}\nend"
        assert items_by_label["apply Type"]["insertText"] == "apply ${1:Tipo}\n    ${0}\nend"
        assert items_by_label["apply Trait to Type"]["insertText"] == "apply ${1:Trait} to ${2:Tipo}\n    ${0}\nend"
        assert items_by_label["fmt"]["insertText"] == 'fmt "${1:texto {valor}}"'
        assert items_by_label["int()"]["insertText"] == "int(${1:value})"
        assert items_by_label["to_text()"]["detail"] == "to_text(value) -> text"
        assert "TextRepresentable<T>" in items_by_label["to_text()"]["documentation"]
        assert "exibicao, logs e mensagens" in items_by_label["to_text()"]["documentation"]
        assert "Assinatura" in items_by_label["fmt"]["documentation"]

        dyn_text = (
            "namespace app.main\n\n"
            "import app.util as util\n\n"
            "func show(item: any<util.Displayable>) -> text\n"
            "    item.\n"
            "    return \"\"\n"
            "end\n"
        )
        send(
            proc,
            {
                "jsonrpc": "2.0",
                "method": "textDocument/didChange",
                "params": {
                    "textDocument": {"uri": uri, "version": 11},
                    "contentChanges": [{"text": dyn_text}],
                },
            },
        )
        expect(messages, errors, "dyn diagnostics")
        send(
            proc,
            {
                "jsonrpc": "2.0",
                "id": 51,
                "method": "textDocument/completion",
                "params": {"textDocument": {"uri": uri}, "position": {"line": 5, "character": 9}},
            },
        )
        dyn_completion = expect(messages, errors, "dyn member completion")
        dyn_labels = {item["label"] for item in dyn_completion["result"]}
        assert "label" in dyn_labels

        inferred_text = (
            "namespace app.main\n\n"
            "import app.util as util\n\n"
            "func inferred() -> int\n"
            "    using box = util.HelperBox(1, \"x\")\n"
            "        box.\n"
            "    end\n"
            "    using values = [1, 2]\n"
            "        values.\n"
            "    end\n"
            "    return 0\n"
            "end\n"
        )
        send(
            proc,
            {
                "jsonrpc": "2.0",
                "method": "textDocument/didChange",
                "params": {
                    "textDocument": {"uri": uri, "version": 12},
                    "contentChanges": [{"text": inferred_text}],
                },
            },
        )
        expect(messages, errors, "inferred diagnostics")
        send(
            proc,
            {
                "jsonrpc": "2.0",
                "id": 52,
                "method": "textDocument/completion",
                "params": {"textDocument": {"uri": uri}, "position": {"line": 6, "character": 12}},
            },
        )
        inferred_struct_completion = expect(messages, errors, "inferred struct completion")
        inferred_struct_labels = {item["label"] for item in inferred_struct_completion["result"]}
        assert "value" in inferred_struct_labels
        assert "label" in inferred_struct_labels
        send(
            proc,
            {
                "jsonrpc": "2.0",
                "id": 53,
                "method": "textDocument/completion",
                "params": {"textDocument": {"uri": uri}, "position": {"line": 9, "character": 15}},
            },
        )
        inferred_list_completion = expect(messages, errors, "inferred list completion")
        inferred_list_labels = {item["label"] for item in inferred_list_completion["result"]}
        assert "get" in inferred_list_labels
        send(
            proc,
            {
                "jsonrpc": "2.0",
                "id": 54,
                "method": "textDocument/hover",
                "params": {"textDocument": {"uri": uri}, "position": {"line": 8, "character": 12}},
            },
        )
        inferred_hover = expect(messages, errors, "inferred hover")
        assert "using values: list<int>" in inferred_hover["result"]["contents"]["value"]

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
        assert "lib.extra" in import_labels
        assert "func" not in import_labels
        import_order = [item["label"] for item in import_completion["result"]]
        assert import_order.index("app.util") < import_order.index("lib.extra")
        assert import_order.index("lib.extra") < import_order.index("std.io")

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
                "id": 67,
                "method": "textDocument/hover",
                "params": {"textDocument": {"uri": uri}, "position": {"line": 5, "character": 17}},
            },
        )
        imported_hover = expect(messages, errors, "imported hover")
        assert "Origem: importado de app.util." in imported_hover["result"]["contents"]["value"]
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

        local_scope_text = (
            "namespace app.main\n\n"
            "func scope(a: int) -> int\n"
            "    var value: int = a\n"
            "    if true\n"
            "        var value: int = 2\n"
            "        value\n"
            "    end\n"
            "    using temp = [1, 2]\n"
            "        temp.\n"
            "    end\n"
            "    return value + a\n"
            "end\n"
        )
        send(
            proc,
            {
                "jsonrpc": "2.0",
                "method": "textDocument/didChange",
                "params": {
                    "textDocument": {"uri": uri, "version": 13},
                    "contentChanges": [{"text": local_scope_text}],
                },
            },
        )
        expect(messages, errors, "local scope diagnostics")

        send(
            proc,
            {
                "jsonrpc": "2.0",
                "id": 70,
                "method": "textDocument/references",
                "params": {
                    "textDocument": {"uri": uri},
                    "position": {"line": 11, "character": 13},
                    "context": {"includeDeclaration": True},
                },
            },
        )
        local_value_refs = expect(messages, errors, "local value references")
        local_value_ref_locations = {
            (item["uri"], item["range"]["start"]["line"], item["range"]["start"]["character"])
            for item in local_value_refs["result"]
        }
        assert local_value_ref_locations == {(uri, 3, 8), (uri, 11, 11)}

        send(
            proc,
            {
                "jsonrpc": "2.0",
                "id": 71,
                "method": "textDocument/rename",
                "params": {
                    "textDocument": {"uri": uri},
                    "position": {"line": 11, "character": 13},
                    "newName": "total",
                },
            },
        )
        local_value_rename = expect(messages, errors, "local value rename")
        local_value_edits = local_value_rename["result"]["changes"][uri]
        local_value_edit_locations = {
            (edit["range"]["start"]["line"], edit["range"]["start"]["character"], edit["newText"])
            for edit in local_value_edits
        }
        assert local_value_edit_locations == {(3, 8, "total"), (11, 11, "total")}

        send(
            proc,
            {
                "jsonrpc": "2.0",
                "id": 72,
                "method": "textDocument/references",
                "params": {
                    "textDocument": {"uri": uri},
                    "position": {"line": 11, "character": 19},
                    "context": {"includeDeclaration": True},
                },
            },
        )
        local_param_refs = expect(messages, errors, "local param references")
        local_param_ref_locations = {
            (item["uri"], item["range"]["start"]["line"], item["range"]["start"]["character"])
            for item in local_param_refs["result"]
        }
        assert local_param_ref_locations == {(uri, 2, 11), (uri, 3, 21), (uri, 11, 19)}

        send(
            proc,
            {
                "jsonrpc": "2.0",
                "id": 73,
                "method": "textDocument/rename",
                "params": {
                    "textDocument": {"uri": uri},
                    "position": {"line": 11, "character": 19},
                    "newName": "input",
                },
            },
        )
        local_param_rename = expect(messages, errors, "local param rename")
        local_param_edits = local_param_rename["result"]["changes"][uri]
        local_param_edit_locations = {
            (edit["range"]["start"]["line"], edit["range"]["start"]["character"], edit["newText"])
            for edit in local_param_edits
        }
        assert local_param_edit_locations == {(2, 11, "input"), (3, 21, "input"), (11, 19, "input")}

        send(
            proc,
            {
                "jsonrpc": "2.0",
                "id": 74,
                "method": "textDocument/references",
                "params": {
                    "textDocument": {"uri": uri},
                    "position": {"line": 9, "character": 9},
                    "context": {"includeDeclaration": True},
                },
            },
        )
        local_using_refs = expect(messages, errors, "local using references")
        local_using_ref_locations = {
            (item["uri"], item["range"]["start"]["line"], item["range"]["start"]["character"])
            for item in local_using_refs["result"]
        }
        assert local_using_ref_locations == {(uri, 8, 10), (uri, 9, 8)}, local_using_ref_locations

        send(
            proc,
            {
                "jsonrpc": "2.0",
                "id": 75,
                "method": "textDocument/rename",
                "params": {
                    "textDocument": {"uri": uri},
                    "position": {"line": 9, "character": 9},
                    "newName": "items",
                },
            },
        )
        local_using_rename = expect(messages, errors, "local using rename")
        local_using_edits = local_using_rename["result"]["changes"][uri]
        local_using_edit_locations = {
            (edit["range"]["start"]["line"], edit["range"]["start"]["character"], edit["newText"])
            for edit in local_using_edits
        }
        assert local_using_edit_locations == {(8, 10, "items"), (9, 8, "items")}

        local_match_text = (
            "namespace app.main\n\n"
            "func matched(input: optional<int>) -> int\n"
            "    match input\n"
            "        case some(found):\n"
            "            return found\n"
            "        case none:\n"
            "            return 0\n"
            "    end\n"
            "end\n"
        )
        send(
            proc,
            {
                "jsonrpc": "2.0",
                "method": "textDocument/didChange",
                "params": {
                    "textDocument": {"uri": uri, "version": 14},
                    "contentChanges": [{"text": local_match_text}],
                },
            },
        )
        expect(messages, errors, "local match diagnostics")

        send(
            proc,
            {
                "jsonrpc": "2.0",
                "id": 76,
                "method": "textDocument/references",
                "params": {
                    "textDocument": {"uri": uri},
                    "position": {"line": 5, "character": 20},
                    "context": {"includeDeclaration": True},
                },
            },
        )
        local_match_refs = expect(messages, errors, "local match references")
        local_match_ref_locations = {
            (item["uri"], item["range"]["start"]["line"], item["range"]["start"]["character"])
            for item in local_match_refs["result"]
        }
        assert local_match_ref_locations == {(uri, 4, 18), (uri, 5, 19)}

        send(
            proc,
            {
                "jsonrpc": "2.0",
                "id": 77,
                "method": "textDocument/rename",
                "params": {
                    "textDocument": {"uri": uri},
                    "position": {"line": 5, "character": 20},
                    "newName": "payload",
                },
            },
        )
        local_match_rename = expect(messages, errors, "local match rename")
        local_match_edits = local_match_rename["result"]["changes"][uri]
        local_match_edit_locations = {
            (edit["range"]["start"]["line"], edit["range"]["start"]["character"], edit["newText"])
            for edit in local_match_edits
        }
        assert local_match_edit_locations == {(4, 18, "payload"), (5, 19, "payload")}

        alias_type_text = (
            "namespace app.main\n\n"
            "type IntValues = list<int>\n\n"
            "func takes(values: IntValues) -> IntValues\n"
            "    return values\n"
            "end\n\n"
            "func alias_member() -> int\n"
            "    var values: IntValues = [1, 2]\n"
            "    values.\n"
            "    takes(values)\n"
            "    return 0\n"
            "end\n"
        )
        send(
            proc,
            {
                "jsonrpc": "2.0",
                "method": "textDocument/didChange",
                "params": {
                    "textDocument": {"uri": uri, "version": 15},
                    "contentChanges": [{"text": alias_type_text}],
                },
            },
        )
        expect(messages, errors, "alias type diagnostics")
        send(
            proc,
            {
                "jsonrpc": "2.0",
                "id": 78,
                "method": "textDocument/completion",
                "params": {"textDocument": {"uri": uri}, "position": {"line": 10, "character": 11}},
            },
        )
        alias_type_completion = expect(messages, errors, "alias type member completion")
        alias_type_labels = {item["label"] for item in alias_type_completion["result"]}
        assert "get" in alias_type_labels

        send(
            proc,
            {
                "jsonrpc": "2.0",
                "id": 79,
                "method": "textDocument/hover",
                "params": {"textDocument": {"uri": uri}, "position": {"line": 9, "character": 9}},
            },
        )
        alias_type_hover = expect(messages, errors, "alias type hover")
        assert "var values: list<int>" in alias_type_hover["result"]["contents"]["value"]

        send(
            proc,
            {
                "jsonrpc": "2.0",
                "id": 80,
                "method": "textDocument/signatureHelp",
                "params": {"textDocument": {"uri": uri}, "position": {"line": 11, "character": 11}},
            },
        )
        alias_type_signature = expect(messages, errors, "alias type signature help")
        alias_type_label = alias_type_signature["result"]["signatures"][0]["label"]
        assert "values: list<int>" in alias_type_label
        assert "-> list<int>" in alias_type_label

        incomplete_alias_text = (
            "namespace app.main\n\n"
            "type IntValues = list<int>\n\n"
            "func incomplete_alias() -> int\n"
            "    var values: IntValues = [1, 2]\n"
            "    values.\n"
        )
        send(
            proc,
            {
                "jsonrpc": "2.0",
                "method": "textDocument/didChange",
                "params": {
                    "textDocument": {"uri": uri, "version": 16},
                    "contentChanges": [{"text": incomplete_alias_text}],
                },
            },
        )
        expect(messages, errors, "incomplete alias diagnostics")
        send(
            proc,
            {
                "jsonrpc": "2.0",
                "id": 81,
                "method": "textDocument/completion",
                "params": {"textDocument": {"uri": uri}, "position": {"line": 6, "character": 11}},
            },
        )
        incomplete_alias_completion = expect(messages, errors, "incomplete alias completion")
        incomplete_alias_labels = {item["label"] for item in incomplete_alias_completion["result"]}
        assert "get" in incomplete_alias_labels

        generic_struct_text = (
            "namespace app.main\n\n"
            "struct GenericBox<T>\n"
            "    item: T\n"
            "end\n\n"
            "func generic_member() -> int\n"
            "    var box: GenericBox<int>\n"
            "    box.\n"
            "    return 0\n"
            "end\n"
        )
        send(
            proc,
            {
                "jsonrpc": "2.0",
                "method": "textDocument/didChange",
                "params": {
                    "textDocument": {"uri": uri, "version": 17},
                    "contentChanges": [{"text": generic_struct_text}],
                },
            },
        )
        expect(messages, errors, "generic struct diagnostics")
        send(
            proc,
            {
                "jsonrpc": "2.0",
                "id": 82,
                "method": "textDocument/completion",
                "params": {"textDocument": {"uri": uri}, "position": {"line": 8, "character": 8}},
            },
        )
        generic_struct_completion = expect(messages, errors, "generic struct member completion")
        generic_items = {item["label"]: item for item in generic_struct_completion["result"]}
        assert "item" in generic_items
        assert generic_items["item"].get("detail") == "GenericBox.item: int"

        enum_completion_text = (
            "namespace app.main\n\n"
            "enum Mode\n"
            "    Ready\n"
            "    Failed(message: text)\n"
            "end\n\n"
            "func enum_member(mode: Mode) -> int\n"
            "    Mode.\n"
            "    match mode\n"
            "        case \n"
            "    end\n"
            "    return 0\n"
            "end\n"
        )
        send(
            proc,
            {
                "jsonrpc": "2.0",
                "method": "textDocument/didChange",
                "params": {
                    "textDocument": {"uri": uri, "version": 18},
                    "contentChanges": [{"text": enum_completion_text}],
                },
            },
        )
        expect(messages, errors, "enum completion diagnostics")
        send(
            proc,
            {
                "jsonrpc": "2.0",
                "id": 83,
                "method": "textDocument/completion",
                "params": {"textDocument": {"uri": uri}, "position": {"line": 8, "character": 9}},
            },
        )
        enum_member_completion = expect(messages, errors, "enum member completion")
        enum_member_items = {item["label"]: item for item in enum_member_completion["result"]}
        assert "Ready" in enum_member_items
        assert "Failed" in enum_member_items
        assert enum_member_items["Failed"].get("insertText") == "Failed(${1:message})"

        send(
            proc,
            {
                "jsonrpc": "2.0",
                "id": 84,
                "method": "textDocument/completion",
                "params": {"textDocument": {"uri": uri}, "position": {"line": 10, "character": 13}},
            },
        )
        enum_case_completion = expect(messages, errors, "enum case completion")
        enum_case_items = {item["label"]: item for item in enum_case_completion["result"]}
        assert "Mode.Ready" in enum_case_items
        assert "Mode.Failed" in enum_case_items
        assert enum_case_items["Mode.Failed"].get("insertText") == "Mode.Failed(${1:message})"

        optional_case_text = (
            "namespace app.main\n\n"
            "func optional_case(input: optional<int>) -> int\n"
            "    match input\n"
            "        case \n"
            "    end\n"
            "    return 0\n"
            "end\n"
        )
        send(
            proc,
            {
                "jsonrpc": "2.0",
                "method": "textDocument/didChange",
                "params": {
                    "textDocument": {"uri": uri, "version": 19},
                    "contentChanges": [{"text": optional_case_text}],
                },
            },
        )
        expect(messages, errors, "optional case diagnostics")
        send(
            proc,
            {
                "jsonrpc": "2.0",
                "id": 85,
                "method": "textDocument/completion",
                "params": {"textDocument": {"uri": uri}, "position": {"line": 4, "character": 13}},
            },
        )
        optional_case_completion = expect(messages, errors, "optional case completion")
        optional_case_items = {item["label"]: item for item in optional_case_completion["result"]}
        assert "some" in optional_case_items
        assert "none" in optional_case_items
        assert "func" not in optional_case_items

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
        send(
            proc,
            {
                "jsonrpc": "2.0",
                "id": 73,
                "method": "textDocument/semanticTokens/full",
                "params": {"textDocument": {"uri": uri}},
            },
        )
        imported_member_semantic = expect(messages, errors, "imported member semantic tokens")
        imported_member_decoded = decode_semantic_tokens(imported_member_text, imported_member_semantic["result"]["data"])
        assert any(item["text"] == "util" and item["type"] == "variable" for item in imported_member_decoded)

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
        send(
            proc,
            {
                "jsonrpc": "2.0",
                "id": 74,
                "method": "textDocument/semanticTokens/full",
                "params": {"textDocument": {"uri": uri}},
            },
        )
        stdlib_member_semantic = expect(messages, errors, "stdlib member semantic tokens")
        stdlib_member_decoded = decode_semantic_tokens(stdlib_member_text, stdlib_member_semantic["result"]["data"])
        assert any(item["text"] == "io" and item["type"] == "variable" for item in stdlib_member_decoded)

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
        assert stdlib_items_by_label["print()"]["insertText"] == "print(${1:value})"
        assert stdlib_items_by_label["print()"]["detail"] == "print(value) -> void"
        assert "Hello from Zenith" in stdlib_items_by_label["print()"]["documentation"]
        assert stdlib_items_by_label["read_line()"]["insertText"] == "io.read_line()"

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
        run_lsp_robustness_smoke()
        print("lsp smoke ok")
        return 0
    finally:
        if proc.poll() is None:
            proc.kill()


if __name__ == "__main__":
    sys.exit(main())
