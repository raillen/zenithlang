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

set "COMMON_FILES="
for /r compiler %%f in (*.c) do (
    set "FILE_NAME=%%~nxf"
    if /I "!FILE_NAME!"=="main.c" (
        rem Skip
    ) else if /I "!FILE_NAME!"=="zpm_main.c" (
        rem Skip
    ) else if /I "!FILE_NAME!"=="lsp.c" (
        rem Skip
    ) else (
        set "COMMON_FILES=!COMMON_FILES! "%%f""
    )
)

if not defined COMMON_FILES (
    echo FAIL
    echo No common C files found under compiler/.
    exit /b 1
)

echo Building zt.exe...
set "ZT_CMD=%CC_BIN% -O0 -Wall -Wextra -I. -o zt.exe compiler\driver\main.c %COMMON_FILES%"
call %ZT_CMD%
set "ZT_RC=%ERRORLEVEL%"

echo Building zpm.exe...
set "ZPM_CMD=%CC_BIN% -O0 -Wall -Wextra -I. -o zpm.exe compiler\driver\zpm_main.c %COMMON_FILES%"
call %ZPM_CMD%
set "ZPM_RC=%ERRORLEVEL%"

if %ZT_RC%==0 if %ZPM_RC%==0 (
    echo SUCCESS
    exit /b 0
)

echo FAIL (zt: %ZT_RC%, zpm: %ZPM_RC%)
exit /b 1
set "RC=%ERRORLEVEL%"

echo Exit code: %RC%
if %RC%==0 (
    echo SUCCESS
) else (
    echo FAIL
)
exit /b %RC%
