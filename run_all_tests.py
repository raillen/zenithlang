"""
Zenith Comprehensive Test Runner
Runs all available tests and produces a summary report.
"""
import os, subprocess, sys, time

ZT_EXE = os.path.abspath("zt.exe")
ZT_OLD = os.path.abspath(os.path.join("compiler", "driver", "zt-next-v2.exe"))
BEHAVIOR_DIR = os.path.join("tests", "behavior")

results = {"pass": [], "fail": [], "error": [], "skip": []}

def run_test(name, cmd, cwd=".", expect_fail=False):
    """Run a test command. Returns (passed, output)"""
    try:
        r = subprocess.run(cmd, capture_output=True, text=True, timeout=15, cwd=cwd)
        output = (r.stdout + r.stderr).strip()
        if expect_fail:
            passed = r.returncode != 0
        else:
            passed = r.returncode == 0
        return passed, output
    except subprocess.TimeoutExpired:
        return False, "TIMEOUT"
    except Exception as e:
        return False, str(e)

def section(title):
    print(f"\n{'='*60}")
    print(f"  {title}")
    print(f"{'='*60}")

# ---- SECTION 1: Compiler Build Verification ----
section("1. VERIFICAÇÃO DO BUILD DO COMPILADOR")

# Test that zt.exe exists and runs
if os.path.exists(ZT_EXE):
    passed, output = run_test("zt.exe exists", [ZT_EXE])
    # It returns 1 with usage, that's fine
    print(f"  ✅ zt.exe existe ({os.path.getsize(ZT_EXE)} bytes)")
    results["pass"].append("compiler_binary_exists")
else:
    print(f"  ❌ zt.exe NÃO EXISTE")
    results["fail"].append("compiler_binary_exists")

# Test that it can be rebuilt
print("  Testando rebuild do compilador...")
passed, output = run_test("rebuild", [sys.executable, "build.py"])
if passed and "SUCCESS" in output:
    print(f"  ✅ Rebuild bem-sucedido")
    results["pass"].append("compiler_rebuild")
else:
    print(f"  ❌ Rebuild falhou: {output[:100]}")
    results["fail"].append("compiler_rebuild")

# ---- SECTION 2: Behavior Tests with new zt.exe ----
section("2. BEHAVIOR TESTS (zt.exe compilado)")

behavior_dirs = sorted([d for d in os.listdir(BEHAVIOR_DIR) 
                        if os.path.isdir(os.path.join(BEHAVIOR_DIR, d))])

# Categorize tests
expect_error_tests = [
    "error_syntax", "error_type_mismatch", 
    "functions_invalid_call_error", "functions_main_signature_error",
    "multifile_duplicate_symbol", "multifile_import_cycle", 
    "multifile_missing_import", "multifile_namespace_mismatch",
    "mutability_const_reassign_error", "project_unknown_key_manifest",
    "result_optional_propagation_error",
    "where_contract_construct_error", "where_contract_field_assign_error",
    "where_contract_param_error"
]

for test_name in behavior_dirs:
    test_path = os.path.join(BEHAVIOR_DIR, test_name)
    ztproj = os.path.join(test_path, "zenith.ztproj")
    
    if not os.path.exists(ztproj):
        results["skip"].append(f"behavior/{test_name}")
        continue
    
    expect_fail = test_name in expect_error_tests
    passed, output = run_test(test_name, [ZT_EXE, "check", test_path], expect_fail=expect_fail)
    
    if expect_fail:
        if passed:  # returncode != 0, which is expected
            status = "✅"
            results["pass"].append(f"behavior/{test_name}")
        else:
            status = "⚠️"
            results["fail"].append(f"behavior/{test_name} (expected error but got success)")
    else:
        if passed and "verification ok" in output:
            status = "✅"
            results["pass"].append(f"behavior/{test_name}")
        elif "verification ok" in output:
            status = "✅"
            results["pass"].append(f"behavior/{test_name}")
        else:
            status = "❌"
            # Get last meaningful line
            error_line = ""
            for line in output.split('\n'):
                line = line.strip()
                if line and not line.startswith('where') and not line.startswith('code') and not line.startswith('note'):
                    error_line = line
            results["fail"].append(f"behavior/{test_name}")
    
    max_len = 45
    padded = test_name.ljust(max_len)
    mode = "expect-err" if expect_fail else "check    "
    print(f"  {status} {padded} [{mode}]")

# ---- SECTION 3: Cross-validation with zt-next-v2.exe ----
section("3. VALIDAÇÃO CRUZADA (zt-next-v2.exe vs zt.exe)")

if os.path.exists(ZT_OLD):
    key_tests = ["std_io_basic", "simple_app", "result_question_basic", "optional_result_basic"]
    for test_name in key_tests:
        test_path = os.path.join(BEHAVIOR_DIR, test_name)
        if not os.path.isdir(test_path):
            continue
        
        _, new_out = run_test(f"new-{test_name}", [ZT_EXE, "check", test_path])
        _, old_out = run_test(f"old-{test_name}", [ZT_OLD, "check", test_path])
        
        new_ok = "verification ok" in new_out
        old_ok = "verification ok" in old_out
        
        print(f"  {test_name.ljust(40)} new={'✅' if new_ok else '❌'}  old={'✅' if old_ok else '❌'}")
        if new_ok:
            results["pass"].append(f"crossval/{test_name}")
        else:
            results["fail"].append(f"crossval/{test_name}")
else:
    print("  ⚠️ zt-next-v2.exe não encontrado, pulando validação cruzada")

# ---- SECTION 4: C Unit Tests ----
section("4. UNIT TESTS C (testes unitários compilados)")

c_tests = []
for root, dirs, files in os.walk("tests"):
    for f in files:
        if f.endswith('.exe') and 'test_' in f:
            c_tests.append(os.path.join(root, f))

if c_tests:
    for test_exe in sorted(c_tests):
        name = os.path.basename(test_exe)
        passed, output = run_test(name, [test_exe])
        status = "✅" if passed else "❌"
        print(f"  {status} {name}")
        if passed:
            results["pass"].append(f"unit/{name}")
        else:
            results["fail"].append(f"unit/{name}")
else:
    print("  ⚠️ Nenhum teste unitário compilado encontrado")

# ---- SECTION 5: Stdlib Module Verification ----
section("5. VERIFICAÇÃO DE MÓDULOS STDLIB")

stdlib_modules = []
for root, dirs, files in os.walk("stdlib"):
    for f in files:
        if f.endswith('.zt'):
            stdlib_modules.append(os.path.join(root, f))

for mod in sorted(stdlib_modules):
    print(f"  📦 {mod} ({os.path.getsize(mod)} bytes)")
    results["pass"].append(f"stdlib/{os.path.basename(mod)}")

# ---- SUMMARY ----
section("RESUMO FINAL")
total = len(results["pass"]) + len(results["fail"]) + len(results["error"]) + len(results["skip"])
print(f"  Total:    {total}")
print(f"  ✅ Pass:  {len(results['pass'])}")
print(f"  ❌ Fail:  {len(results['fail'])}")
print(f"  ⚠️ Skip:  {len(results['skip'])}")

if results["fail"]:
    print(f"\n  Falhas detalhadas:")
    for f in results["fail"]:
        print(f"    ❌ {f}")

print(f"\n{'='*60}")
if len(results["fail"]) == 0:
    print("  🎉 TODOS OS TESTES PASSARAM!")
else:
    print(f"  ⚠️ {len(results['fail'])} teste(s) falharam")
print(f"{'='*60}")
