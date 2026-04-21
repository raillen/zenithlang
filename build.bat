@echo off
setlocal EnableExtensions EnableDelayedExpansion

set "CC_BIN="
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
    echo FAIL
    echo No C compiler found. Install gcc or clang, or set CC.
    exit /b 1
)

set "C_FILES="
for /r compiler %%f in (*.c) do (
    if /I not "%%~nxf"=="lsp.c" (
        set "C_FILES=!C_FILES! "%%f""
    )
)

if not defined C_FILES (
    echo FAIL
    echo No C files found under compiler/.
    exit /b 1
)

set "CMD=%CC_BIN% -O0 -Wall -Wextra -I. -o zt.exe%C_FILES%"
echo Building with: %CMD%
call %CMD%
set "RC=%ERRORLEVEL%"

echo Exit code: %RC%
if %RC%==0 (
    echo SUCCESS
) else (
    echo FAIL
)
exit /b %RC%
