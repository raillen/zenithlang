@echo off
setlocal EnableExtensions EnableDelayedExpansion

echo ====================================
echo Compilando testes R2.M3
echo ====================================

if defined CC (
    set "CC_BIN=%CC%"
) else (
    where gcc >nul 2>nul
    if %ERRORLEVEL%==0 (
        set "CC_BIN=gcc"
    ) else (
        where clang >nul 2>nul
        if %ERRORLEVEL%==0 (
            set "CC_BIN=clang"
        )
    )
)

if not defined CC_BIN (
    echo ERRO: Nenhum compilador C encontrado. Instale gcc/clang ou defina CC.
    exit /b 1
)

set "CFLAGS=-O0 -Wall -Wextra -I."
set "RSP=%TEMP%\zenith_r2m3_compiler_sources.rsp"

if exist "%RSP%" del /f /q "%RSP%" >nul 2>nul

for /r compiler %%f in (*.c) do (
    set "SRC=%%f"
    if /I "!SRC:\compiler\driver\=!"=="!SRC!" (
        >> "%RSP%" echo "%%f"
    )
)

if not exist "%RSP%" (
    echo ERRO: lista de fontes nao foi gerada.
    exit /b 1
)

call :build_one tests\semantic\test_optional_result_properties.c tests\semantic\test_optional_result_properties.exe test_optional_result_properties
if errorlevel 1 exit /b 1

call :build_one tests\semantic\test_where_contract_properties.c tests\semantic\test_where_contract_properties.exe test_where_contract_properties
if errorlevel 1 exit /b 1

call :build_one tests\semantic\test_conversion_overflow_properties.c tests\semantic\test_conversion_overflow_properties.exe test_conversion_overflow_properties
if errorlevel 1 exit /b 1

del /f /q "%RSP%" >nul 2>nul

echo.
echo ====================================
echo Todos os testes compilados com sucesso!
echo ====================================
exit /b 0

:build_one
set "SRC=%~1"
set "OUT=%~2"
set "NAME=%~3"

echo.
echo Compilando %NAME%...
"%CC_BIN%" %CFLAGS% -o "%OUT%" "%SRC%" @"%RSP%"
if errorlevel 1 (
    echo ERRO: Falha ao compilar %NAME%
    exit /b 1
)
echo OK!
exit /b 0
