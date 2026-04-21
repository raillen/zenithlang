# R2.M3 - Test Runner para testes Zenith
# Executa zt.exe check em cada teste e valida os resultados

$zt_exe = ".\zt.exe"
$tests_passed = 0
$tests_failed = 0
$tests_total = 0

Write-Host "====================================" -ForegroundColor Cyan
Write-Host "R2.M3 - Test Runner (Zenith Tests)" -ForegroundColor Cyan
Write-Host "====================================" -ForegroundColor Cyan
Write-Host ""

# Função para executar um teste
function Run-Test {
    param(
        [string]$TestName,
        [string]$TestFile,
        [bool]$ShouldPass
    )
    
    $script:tests_total++
    Write-Host "[$script:tests_total] Testing: $TestName" -NoNewline
    
    # Executar zt.exe check
    $output = & $zt_exe check $TestFile 2>&1
    $exit_code = $LASTEXITCODE
    
    if ($ShouldPass -and $exit_code -eq 0) {
        Write-Host " ✅ PASS" -ForegroundColor Green
        $script:tests_passed++
        return $true
    }
    elseif (-not $ShouldPass -and $exit_code -ne 0) {
        Write-Host " ✅ PASS (expected failure)" -ForegroundColor Green
        $script:tests_passed++
        return $true
    }
    else {
        Write-Host " ❌ FAIL" -ForegroundColor Red
        Write-Host "    Exit code: $exit_code"
        Write-Host "    Output: $output"
        $script:tests_failed++
        return $false
    }
}

# Testes de Optional/Result
Write-Host "`n--- Optional/Result Tests ---" -ForegroundColor Yellow

# Criar arquivo temporário para cada teste
$tempDir = Join-Path $env:TEMP "zenith_r2m3_tests"
if (-not (Test-Path $tempDir)) {
    New-Item -ItemType Directory -Path $tempDir | Out-Null
}

# TEST 1: Optional null (deve passar)
$test1 = Join-Path $tempDir "test1_optional_null.zt"
@"
namespace test1
func demo() -> int?
    return null
end
"@ | Out-File -FilePath $test1 -Encoding UTF8
Run-Test "Optional accepts null" $test1 $true

# TEST 2: Optional value (deve passar)
$test2 = Join-Path $tempDir "test2_optional_value.zt"
@"
namespace test2
func demo() -> int?
    return 42
end
"@ | Out-File -FilePath $test2 -Encoding UTF8
Run-Test "Optional accepts value" $test2 $true

# TEST 3: Null to non-optional (deve falhar)
$test3 = Join-Path $tempDir "test3_null_fail.zt"
@"
namespace test3
func demo() -> int
    return null
end
"@ | Out-File -FilePath $test3 -Encoding UTF8
Run-Test "Null rejected in non-optional" $test3 $false

# TEST 4: Type mismatch (deve falhar)
$test4 = Join-Path $tempDir "test4_type_mismatch.zt"
@"
namespace test4
func demo() -> int
    return "texto"
end
"@ | Out-File -FilePath $test4 -Encoding UTF8
Run-Test "Type mismatch detected" $test4 $false

# TEST 5: Unresolved name (deve falhar)
$test5 = Join-Path $tempDir "test5_unresolved.zt"
@"
namespace test5
func demo() -> int
    return undefined_var
end
"@ | Out-File -FilePath $test5 -Encoding UTF8
Run-Test "Unresolved name detected" $test5 $false

# Resultado final
Write-Host ""
Write-Host "====================================" -ForegroundColor Cyan
Write-Host "Resultados:" -ForegroundColor Cyan
Write-Host "====================================" -ForegroundColor Cyan
Write-Host "Total:   $tests_total"
Write-Host "Passed:  $tests_passed" -ForegroundColor Green
Write-Host "Failed:  $tests_failed" -ForegroundColor $(if ($tests_failed -eq 0) { "Green" } else { "Red" })
Write-Host ""

if ($tests_failed -eq 0) {
    Write-Host "✅ TODOS OS TESTES PASSARAM!" -ForegroundColor Green
    exit 0
} else {
    Write-Host "❌ ALGUNS TESTES FALHARAM" -ForegroundColor Red
    exit 1
}
