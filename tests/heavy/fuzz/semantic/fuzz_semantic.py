#!/usr/bin/env python3
"""
Semantic Layer Fuzzer — Zenith Language

Gera programas Zenith válidos sintaticamente mas com tipos/semântica absurda
para encontrar bugs no type checker, binder e semantic analysis.
"""

import os
import sys
import random
import string
import subprocess
import argparse
import time
import re
from pathlib import Path
from typing import List, Tuple, Optional
from datetime import datetime

ROOT_DIR = Path(__file__).resolve().parents[4]
ZT_PATH = ROOT_DIR / "zt.exe"
FUZZ_DIR = ROOT_DIR / "tests" / "heavy" / "fuzz" / "semantic"
CRASH_DIR = FUZZ_DIR / "crashes"
HANG_DIR = FUZZ_DIR / "hangs"
NAMESPACE_RE = re.compile(r"^\s*namespace\s+([A-Za-z_][A-Za-z0-9_\.]*)\s*$", re.MULTILINE)


class SemanticFuzzer:
    def __init__(self, seed: int = None, timeout: float = 10.0):
        self.seed = seed if seed is not None else int(time.time())
        random.seed(self.seed)
        self.timeout = timeout
        self.iterations = 0
        self.crashes = 0
        self.hangs = 0
        self.successes = 0
        self.errors = 0

        # Tipos para fuzzing
        self.types = [
            "int",
            "float",
            "text",
            "bytes",
            "bool",
            "void",
            "int8",
            "int16",
            "int32",
            "int64",
            "u8",
            "u16",
            "u32",
            "u64",
            "float32",
            "float64",
        ]

        self.generic_types = [
            "list<int>",
            "list<text>",
            "map<text,int>",
            "optional<int>",
            "result<int,text>",
        ]

        # Nomes aleatórios
        self.names = [
            "foo",
            "bar",
            "baz",
            "qux",
            "quux",
            "corge",
            "grault",
            "garply",
            "waldo",
            "fred",
            "plugh",
        ]

    def random_string(self, length: int = 8) -> str:
        return "".join(random.choices(string.ascii_lowercase, k=length))

    def random_name(self) -> str:
        return random.choice(self.names) + "_" + self.random_string(4)

    def random_type(self) -> str:
        if random.random() < 0.3:
            return random.choice(self.generic_types)
        return random.choice(self.types)

    # ============== FUZZ GENERATORS ==============

    def fuzz_type_mismatch(self) -> str:
        """Gera mismatch de tipos absurdo"""
        name = self.random_name()
        t1 = self.random_type()
        t2 = self.random_type()

        return f"""namespace fuzz.type_mismatch

public func main() -> int
    const {name}: {t1} = "definitely_not_{t1}"
    const result: {t2} = {name}
    return 0
end
"""

    def fuzz_circular_type(self) -> str:
        """Gera tipos circulares"""
        return f"""namespace fuzz.circular_type

struct NodeA
    ref: optional<NodeB>
end

struct NodeB
    ref: optional<NodeA>
end

public func main() -> int
    var a: NodeA = NodeA(ref: none)
    var b: NodeB = NodeB(ref: none)
    a.ref = some(b)
    b.ref = some(a)
    return 0
end
"""

    def fuzz_generic_overflow(self) -> str:
        """Gera tipos genéricos profundamente aninhados"""
        depth = random.randint(10, 50)
        base_type = "int"

        nested = base_type
        for _ in range(depth):
            nested = f"list<{nested}>"

        return f"""namespace fuzz.generic_overflow

public func main() -> int
    const x: {nested} = []
    return len(x)
end
"""

    def fuzz_trait_violation(self) -> str:
        """Gera violação de trait constraint"""
        return f"""namespace fuzz.trait_violation

struct NotHashable
    value: int
end

public func main() -> int
    -- map requer Hashable, NotHashable não implementa
    const m: map<NotHashable, int> = {{}}
    return 0
end
"""

    def fuzz_mutability_violation(self) -> str:
        """Gera violação de mutabilidade"""
        return f"""namespace fuzz.mutability_violation

struct Immutable
    value: int
end

public func main() -> int
    const x: Immutable = Immutable(value: 42)
    x.value = 100  -- Error: const struct
    return x.value
end
"""

    def fuzz_optional_abuse(self) -> str:
        """Gera abuso de optional"""
        return f"""namespace fuzz.optional_abuse

public func main() -> int
    const maybe: optional<int> = none
    -- Tenta usar optional como valor direto
    const result: int = maybe + 10
    return result
end
"""

    def fuzz_result_propagation(self) -> str:
        """Gera propagação incorreta de result"""
        return f"""namespace fuzz.result_propagation

func might_fail() -> result<int, text>
    return error("failed")
end

public func main() -> int
    -- ? fora de contexto result
    const value: int = might_fail()
    return value
end
"""

    def fuzz_where_contract(self) -> str:
        """Gera contrato where impossível"""
        return f"""namespace fuzz.where_contract

public func main() -> int
    -- Contrato onde é impossível satisfazer
    const x: int where it > 10 and it < 5 = 7
    return x
end
"""

    def fuzz_self_reference(self) -> str:
        """Gera auto-referência em struct"""
        return f"""namespace fuzz.self_reference

struct SelfRef
    value: int
    ref: SelfRef  -- Struct contém a si mesma (não opcional)
end

public func main() -> int
    const s: SelfRef = SelfRef(value: 1, ref: ???)
    return 0
end
"""

    def fuzz_method_receiver(self) -> str:
        """Gera erro de receiver mutante"""
        return f"""namespace fuzz.method_receiver

struct Counter
    value: int
end

apply Counter
    mut func increment()
        self.value = self.value + 1
    end
end

public func main() -> int
    const c: Counter = Counter(value: 0)
    c.increment()  -- Error: const receiver, mut method
    return c.value
end
"""

    def fuzz_import_cycle(self) -> Tuple[str, str]:
        """Gera ciclo de imports"""
        mod_a = f"""namespace fuzz.cycle.a

import fuzz.cycle.b as b

public func get_a() -> int
    return b.get_b() + 1
end
"""
        mod_b = f"""namespace fuzz.cycle.b

import fuzz.cycle.a as a

public func get_b() -> int
    return a.get_a() + 1
end
"""
        return mod_a, mod_b

    def fuzz_duplicate_symbol(self) -> str:
        """Gera símbolo duplicado"""
        name = self.random_name()
        return f"""namespace fuzz.duplicate

func {name}() -> int
    return 1
end

func {name}(x: int) -> int  -- Duplicate
    return x
end

public func main() -> int
    return {name}()
end
"""

    def fuzz_unbound_type(self) -> str:
        """Gera tipo não vinculado"""
        return f"""namespace fuzz.unbound_type

public func main() -> int
    const x: NonExistentType = ???
    return 0
end
"""

    def fuzz_array_overflow(self) -> str:
        """Gera struct com >128 campos (overflow used_params)"""
        fields = ",\n    ".join([f"field{i}: int" for i in range(150)])
        return f"""namespace fuzz.array_overflow

struct MegaStruct
    {fields}
end

public func main() -> int
    const s: MegaStruct = MegaStruct(
        {", ".join([f"field{i}: {i}" for i in range(150)])}
    )
    return s.field0
end
"""

    def fuzz_deep_if(self) -> str:
        """Gera if/else profundamente aninhado"""
        depth = random.randint(50, 200)
        content = "namespace fuzz.deep_if\n\npublic func main() -> int\n"

        for i in range(depth):
            content += f"    if {i} == {i}\n"

        content += f"        return {depth}\n"

        for _ in range(depth):
            content += "    else\n    end\n"

        content += "    return 0\nend\n"
        return content

    def fuzz_fmt_interpolation(self) -> str:
        """Gera interpolação fmt malformada"""
        return f"""namespace fuzz.fmt_interp

public func main() -> int
    -- Chaves desbalanceadas
    const x: text = fmt "Hello {{name"
    return 0
end
"""

    def fuzz_hex_bytes_overflow(self) -> str:
        """Gera hex bytes gigante (>1024 bytes)"""
        hex_pairs = "DE " * 600  # 600 pares = 1200+ chars
        return f"""namespace fuzz.hex_overflow

const data: bytes = hex bytes "{hex_pairs.strip()}"

public func main() -> int
    return len(data)
end
"""

    def fuzz_large_int_literal(self) -> str:
        """Gera literal int gigante"""
        return f"""namespace fuzz.large_int

public func main() -> int
    -- Literal maior que i64 max
    const x: int = 9223372036854775808
    return x
end
"""

    def fuzz_float_overflow(self) -> str:
        """Gera literal float overflow"""
        return f"""namespace fuzz.float_overflow

public func main() -> int
    const x: float = 1e309
    return to_int(x)
end
"""

    # ============== FUZZ EXECUTION ==============

    def extract_namespace(self, source: str) -> str:
        match = NAMESPACE_RE.search(source)
        if match is None:
            raise ValueError("generated fuzz source is missing a namespace declaration")
        return match.group(1)

    def run_fuzz(self, source: str, test_name: str) -> str:
        """Executa fuzz test e retorna status"""
        test_dir = FUZZ_DIR / test_name
        test_dir.mkdir(parents=True, exist_ok=True)

        namespace = self.extract_namespace(source)
        relative_source = Path("src").joinpath(*namespace.split(".")).with_suffix(".zt")
        source_path = test_dir / relative_source
        source_path.parent.mkdir(parents=True, exist_ok=True)
        source_path.write_text(source, encoding="utf-8")

        proj = "\n".join(
            [
                "[project]",
                f'name = "{test_name}"',
                'kind = "app"',
                'version = "0.1.0"',
                "",
                "[source]",
                'root = "src"',
                "",
                "[app]",
                f'entry = "{namespace}"',
                "",
                "[build]",
                'target = "native"',
                'output = "build"',
                'profile = "debug"',
                "",
            ]
        )
        (test_dir / "zenith.ztproj").write_text(proj, encoding="utf-8")

        try:
            result = subprocess.run(
                [str(ZT_PATH), "check", str(test_dir / "zenith.ztproj")],
                capture_output=True,
                text=True,
                encoding="utf-8",
                errors="replace",
                timeout=self.timeout,
                cwd=str(ROOT_DIR),
            )

            stderr_text = result.stderr or ""

            if result.returncode == 0:
                # Compiler aceitou código que deveria falhar
                return "ACCEPTED_UNEXPECTED"
            elif "panic" in stderr_text.lower() or "crash" in stderr_text.lower():
                return "CRASH"
            elif result.returncode != 0:
                return "REJECTED_OK"
            else:
                return "UNKNOWN"

        except subprocess.TimeoutExpired:
            return "HANG"
        except Exception as e:
            return f"ERROR: {e}"

    def save_crash(self, source: str, test_name: str, error: str):
        """Salva caso de crash para análise"""
        CRASH_DIR.mkdir(parents=True, exist_ok=True)
        timestamp = datetime.now().strftime("%Y%m%d-%H%M%S")
        crash_file = CRASH_DIR / f"{test_name}-{timestamp}.zt"

        with open(crash_file, "w", encoding="utf-8") as f:
            f.write(f"-- CRASH: {error}\n")
            f.write(f"-- Seed: {self.seed}\n")
            f.write(f"-- Iteration: {self.iterations}\n\n")
            f.write(source)

    def save_hang(self, source: str, test_name: str):
        """Salva caso de hang para análise"""
        HANG_DIR.mkdir(parents=True, exist_ok=True)
        timestamp = datetime.now().strftime("%Y%m%d-%H%M%S")
        hang_file = HANG_DIR / f"{test_name}-{timestamp}.zt"

        with open(hang_file, "w", encoding="utf-8") as f:
            f.write(f"-- HANG (timeout > {self.timeout}s)\n")
            f.write(f"-- Seed: {self.seed}\n")
            f.write(f"-- Iteration: {self.iterations}\n\n")
            f.write(source)

    def run_fuzz_campaign(self, iterations: int = 100):
        """Executa campanha de fuzzing"""
        generators = [
            ("type_mismatch", self.fuzz_type_mismatch),
            ("circular_type", self.fuzz_circular_type),
            ("trait_violation", self.fuzz_trait_violation),
            ("mutability_violation", self.fuzz_mutability_violation),
            ("optional_abuse", self.fuzz_optional_abuse),
            ("result_propagation", self.fuzz_result_propagation),
            ("where_contract", self.fuzz_where_contract),
            ("self_reference", self.fuzz_self_reference),
            ("method_receiver", self.fuzz_method_receiver),
            ("duplicate_symbol", self.fuzz_duplicate_symbol),
            ("unbound_type", self.fuzz_unbound_type),
            ("array_overflow", self.fuzz_array_overflow),
            ("fmt_interpolation", self.fuzz_fmt_interpolation),
            ("hex_bytes_overflow", self.fuzz_hex_bytes_overflow),
            ("large_int_literal", self.fuzz_large_int_literal),
            ("float_overflow", self.fuzz_float_overflow),
        ]

        print(f"\n{'=' * 60}")
        print("Semantic Fuzzer - Zenith Language")
        print(f"Seed: {self.seed}")
        print(f"Iterations: {iterations}")
        print(f"Timeout: {self.timeout}s")
        print(f"{'=' * 60}\n")

        for i in range(iterations):
            self.iterations = i + 1

            # Escolhe gerador aleatório
            test_name, generator = random.choice(generators)

            # Gera código
            if test_name == "import_cycle":
                mod_a, mod_b = generator()
                # Implementação especial para multi-file
                continue

            source = generator()

            # Executa fuzz
            status = self.run_fuzz(source, f"{test_name}_{i}")

            if status == "CRASH":
                self.crashes += 1
                self.save_crash(source, test_name, "Compiler crash detected")
                print(f"[{i + 1}/{iterations}] {test_name}: FAIL CRASH")
            elif status == "HANG":
                self.hangs += 1
                self.save_hang(source, test_name)
                print(f"[{i + 1}/{iterations}] {test_name}: WARN HANG")
            elif status == "ACCEPTED_UNEXPECTED":
                self.crashes += 1
                self.save_crash(source, test_name, "Accepted unexpected code")
                print(f"[{i + 1}/{iterations}] {test_name}: FAIL ACCEPTED_UNEXPECTED")
            elif status == "REJECTED_OK":
                self.successes += 1
                print(f"[{i + 1}/{iterations}] {test_name}: OK REJECTED_OK")
            else:
                self.errors += 1
                print(f"[{i + 1}/{iterations}] {test_name}: ? {status}")

        # Report
        print(f"\n{'=' * 60}")
        print(f"Fuzz Campaign Results")
        print(f"{'=' * 60}")
        print(f"Total: {self.iterations}")
        print(f"Successes (rejected bad code): {self.successes}")
        print(f"Crashes: {self.crashes}")
        print(f"Hangs: {self.hangs}")
        print(f"Errors: {self.errors}")

        if self.crashes > 0 or self.hangs > 0:
            print("\nWARN Crashes/Hangs saved to:")
            print(f"   - {CRASH_DIR}")
            print(f"   - {HANG_DIR}")

        print(f"{'=' * 60}\n")


def main():
    parser = argparse.ArgumentParser(description="Semantic Fuzzer for Zenith")
    parser.add_argument(
        "--iterations", "--iters", dest="iterations", type=int, default=100, help="Number of fuzz iterations"
    )
    parser.add_argument(
        "--seed", type=int, default=None, help="Random seed (default: time-based)"
    )
    parser.add_argument(
        "--timeout", type=float, default=10.0, help="Timeout per test in seconds"
    )

    args = parser.parse_args()

    fuzzer = SemanticFuzzer(seed=args.seed, timeout=args.timeout)
    fuzzer.run_fuzz_campaign(iterations=args.iterations)

    sys.exit(0 if fuzzer.crashes == 0 and fuzzer.hangs == 0 and fuzzer.errors == 0 else 1)


if __name__ == "__main__":
    main()
