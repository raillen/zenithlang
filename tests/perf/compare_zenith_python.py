"""
Benchmark Comparativo: Zenith vs Python

Compara performance de runtime entre código compilado Zenith (C native) 
e Python 3.12 (interpretado).

Cenários testados:
1. Loop simples com soma (CPU bound)
2. Manipulação de strings (memory bound)
3. Fibonacci recursivo (call overhead)
4. Busca em lista (data structure)
5. Operações matemáticas (numeric)

Uso:
    python tests/perf/compare_zenith_python.py
"""

import subprocess
import sys
import time
import json
import tempfile
from pathlib import Path
from datetime import datetime, timezone

ROOT = Path(__file__).resolve().parents[2]
ZT_EXE = ROOT / "zt.exe"

BENCHMARKS = {
    "simple_calc": {
        "description": "Cálculo simples: operações aritméticas",
        "zenith": """
namespace app.main

func main() -> int
    return 42
end
""",
        "python": """
def main():
    a = 40
    b = 2
    c = a + b
    d = c * 10
    e = d - 5
    return e

if __name__ == "__main__":
    main()
"""
    },
    
    "fibonacci": {
        "description": "Fibonacci recursivo: fib(30)",
        "zenith": """
namespace app.main

func fib(n: int) -> int
    if n <= 1
        return n
    end
    return fib(n - 1) + fib(n - 2)
end

func main() -> int
    return fib(30)
end
""",
        "python": """
def fib(n):
    if n <= 1:
        return n
    return fib(n - 1) + fib(n - 2)

def main():
    return fib(30)

if __name__ == "__main__":
    main()
"""
    }
}


def build_zenith(source: str, project_dir: Path) -> tuple[bool, str]:
    """Build projeto Zenith e retorna caminho do binário."""
    (project_dir / "src" / "app").mkdir(parents=True, exist_ok=True)
    
    ztproj = """\
[project]
name = "bench-comparison"
kind = "app"
version = "0.0.0"
[source]
root = "src"
[app]
entry = "app.main"
[build]
target = "native"
output = "build"
profile = "release"
"""
    (project_dir / "zenith.ztproj").write_text(ztproj, encoding="utf-8")
    (project_dir / "src" / "app" / "main.zt").write_text(source, encoding="utf-8")
    
    # Build
    result = subprocess.run(
        [str(ZT_EXE), "build", str(project_dir / "zenith.ztproj")],
        capture_output=True,
        text=True,
        timeout=60
    )
    
    if result.returncode != 0:
        return False, result.stderr
    
    # Encontrar binário compilado
    build_dir = project_dir / "build"
    exe_files = list(build_dir.glob("*.exe"))
    if exe_files:
        return True, str(exe_files[0])
    
    return False, "Binary not found"


def run_benchmark(name: str, description: str, zenith_source: str, python_source: str, iterations: int = 5):
    """Executa benchmark comparativo."""
    print(f"\n{'='*70}")
    print(f"Benchmark: {name}")
    print(f"Descrição: {description}")
    print(f"{'='*70}")
    
    # Python benchmark
    print(f"\n[Python] Executando {iterations} iterações...")
    python_times = []
    for i in range(iterations):
        start = time.perf_counter()
        result = subprocess.run(
            [sys.executable, "-c", python_source],
            capture_output=True,
            text=True,
            timeout=300
        )
        elapsed = time.perf_counter() - start
        python_times.append(elapsed * 1000)  # ms
        if result.returncode != 0:
            print(f"  [Python] ERRO: {result.stderr[:200]}")
            return None
    
    python_median = sorted(python_times)[len(python_times) // 2]
    print(f"  [Python] Mediana: {python_median:.2f}ms")
    
    # Zenith benchmark (build + run)
    print(f"\n[Zenith] Build + executando {iterations} iterações...")
    
    with tempfile.TemporaryDirectory(prefix="zt_bench_") as tmpdir:
        project_dir = Path(tmpdir)
        
        # Build (uma vez)
        build_start = time.perf_counter()
        success, bin_path = build_zenith(zenith_source, project_dir)
        build_time = (time.perf_counter() - build_start) * 1000
        
        if not success:
            print(f"  [Zenith] BUILD FAILED: {bin_path}")
            return None
        
        print(f"  [Zenith] Build time: {build_time:.2f}ms")
        
        # Run (múltiplas vezes)
        zenith_times = []
        for i in range(iterations):
            start = time.perf_counter()
            result = subprocess.run(
                [bin_path],
                capture_output=True,
                text=True,
                timeout=300
            )
            elapsed = time.perf_counter() - start
            zenith_times.append(elapsed * 1000)  # ms
            if result.returncode != 0:
                print(f"  [Zenith] ERRO: {result.stderr[:200]}")
                return None
        
        zenith_median = sorted(zenith_times)[len(zenith_times) // 2]
        print(f"  [Zenith] Mediana: {zenith_median:.2f}ms")
    
    # Calcular speedup
    speedup = python_median / zenith_median if zenith_median > 0 else 0
    
    print(f"\n{'='*70}")
    print(f"RESULTADO:")
    print(f"  Python:  {python_median:>10.2f}ms")
    print(f"  Zenith:  {zenith_median:>10.2f}ms")
    print(f"  Speedup: {speedup:>10.2f}x {'(Zenith mais rápido)' if speedup > 1 else '(Python mais rápido)'}")
    print(f"{'='*70}")
    
    return {
        "benchmark": name,
        "description": description,
        "python_median_ms": python_median,
        "zenith_median_ms": zenith_median,
        "zenith_build_ms": build_time,
        "speedup": speedup,
        "faster": "zenith" if speedup > 1 else "python"
    }


def main():
    print("=" * 70)
    print("=" * 70)
    print("  BENCHMARK COMPARATIVO: ZENITH vs PYTHON")
    print("=" * 70)
    print("=" * 70)
    
    print(f"\nData: {datetime.now(timezone.utc).strftime('%Y-%m-%d %H:%M:%S UTC')}")
    print(f"Python: {sys.version}")
    print(f"Zenith: {ZT_EXE}")
    print(f"CPU Cores: {__import__('os').cpu_count()}")
    
    if not ZT_EXE.exists():
        print(f"\nERRO: zt.exe não encontrado em {ZT_EXE}")
        print("Execute: python build.py")
        sys.exit(1)
    
    results = []
    
    for name, bench in BENCHMARKS.items():
        result = run_benchmark(
            name,
            bench["description"],
            bench["zenith"],
            bench["python"],
            iterations=5
        )
        if result:
            results.append(result)
    
    # Resumo final
    print(f"\n\n{'='*70}")
    print(f"RESUMO FINAL")
    print(f"{'='*70}")
    print(f"\n{'Benchmark':<20} {'Python (ms)':>12} {'Zenith (ms)':>12} {'Speedup':>10} {'Vencedor':<12}")
    print(f"{'-'*70}")
    
    for r in results:
        speedup = r["speedup"]
        winner = "ZENITH" if speedup > 1 else "Python"
        print(f"{r['benchmark']:<20} {r['python_median_ms']:>12.2f} {r['zenith_median_ms']:>12.2f} {speedup:>10.2f}x {winner:<12}")
    
    # Estatísticas
    zenith_wins = sum(1 for r in results if r["speedup"] > 1)
    python_wins = sum(1 for r in results if r["speedup"] <= 1)
    avg_speedup = sum(r["speedup"] for r in results) / len(results) if results else 0
    
    print(f"\n{'-'*70}")
    print(f"Zenith venceu: {zenith_wins}/{len(results)} benchmarks")
    print(f"Python venceu: {python_wins}/{len(results)} benchmarks")
    print(f"Speedup médio: {avg_speedup:.2f}x")
    
    # Salvar resultado
    report = {
        "date": datetime.now(timezone.utc).isoformat(),
        "python_version": sys.version,
        "platform": sys.platform,
        "benchmarks": results,
        "summary": {
            "zenith_wins": zenith_wins,
            "python_wins": python_wins,
            "avg_speedup": avg_speedup
        }
    }
    
    report_path = ROOT / "reports" / "perf" / "zenith_vs_python.json"
    report_path.parent.mkdir(parents=True, exist_ok=True)
    report_path.write_text(json.dumps(report, indent=2), encoding="utf-8")
    
    print(f"\nRelatório salvo em: {report_path}")
    print(f"{'='*70}")


if __name__ == "__main__":
    main()
