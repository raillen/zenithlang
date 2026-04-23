@echo off
echo ====================================
echo Compilando testes R2.M3
echo ====================================

set CC=gcc
set CFLAGS=-O0 -Wall -Wextra -I.

echo.
echo [1/3] Compilando test_optional_result_properties.exe...
%CC% %CFLAGS% -o tests\semantic\test_optional_result_properties.exe ^
    tests\semantic\test_optional_result_properties.c ^
    compiler\utils\arena.c ^
    compiler\utils\string_pool.c ^
    compiler\frontend\lexer\lexer.c ^
    compiler\frontend\parser\parser.c ^
    compiler\semantic\binder\binder.c ^
    compiler\semantic\types\checker.c ^
    compiler\semantic\types\types.c ^
    compiler\semantic\diagnostics\diagnostics.c ^
    compiler\semantic\symbols\symbols.c
if errorlevel 1 (
    echo ERRO: Falha ao compilar test_optional_result_properties
    exit /b 1
)
echo OK!

echo.
echo [2/3] Compilando test_where_contract_properties.exe...
%CC% %CFLAGS% -o tests\semantic\test_where_contract_properties.exe ^
    tests\semantic\test_where_contract_properties.c ^
    compiler\utils\arena.c ^
    compiler\utils\string_pool.c ^
    compiler\frontend\lexer\lexer.c ^
    compiler\frontend\parser\parser.c ^
    compiler\semantic\binder\binder.c ^
    compiler\semantic\types\checker.c ^
    compiler\semantic\types\types.c ^
    compiler\semantic\diagnostics\diagnostics.c ^
    compiler\semantic\symbols\symbols.c
if errorlevel 1 (
    echo ERRO: Falha ao compilar test_where_contract_properties
    exit /b 1
)
echo OK!

echo.
echo [3/3] Compilando test_conversion_overflow_properties.exe...
%CC% %CFLAGS% -o tests\semantic\test_conversion_overflow_properties.exe ^
    tests\semantic\test_conversion_overflow_properties.c ^
    compiler\utils\arena.c ^
    compiler\utils\string_pool.c ^
    compiler\frontend\lexer\lexer.c ^
    compiler\frontend\parser\parser.c ^
    compiler\semantic\binder\binder.c ^
    compiler\semantic\types\checker.c ^
    compiler\semantic\types\types.c ^
    compiler\semantic\diagnostics\diagnostics.c ^
    compiler\semantic\symbols\symbols.c
if errorlevel 1 (
    echo ERRO: Falha ao compilar test_conversion_overflow_properties
    exit /b 1
)
echo OK!

echo.
echo ====================================
echo Todos os testes compilados com sucesso!
echo ====================================
