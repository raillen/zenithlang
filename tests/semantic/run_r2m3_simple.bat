@echo off
echo ================================
echo R2.M3 Test Runner - Simple
echo ================================
echo.

set PASS=0
set FAIL=0
set TOTAL=0

:: Test 1: Optional null (should pass)
echo [1] Testing: Optional accepts null
mkdir %TEMP%\test1_proj\src\app 2>nul
echo name=t1 > %TEMP%\test1_proj\zenith.ztproj
echo version=0.1.0 >> %TEMP%\test1_proj\zenith.ztproj
echo source_root=src >> %TEMP%\test1_proj\zenith.ztproj
echo namespace app > %TEMP%\test1_proj\src\app\main.zt
echo func demo() -^> int? >> %TEMP%\test1_proj\src\app\main.zt
echo     return null >> %TEMP%\test1_proj\src\app\main.zt
echo end >> %TEMP%\test1_proj\src\app\main.zt
zt.exe check %TEMP%\test1_proj >nul 2>&1
if %errorlevel% equ 0 (
    echo    PASS
    set /a PASS+=1
) else (
    echo    FAIL
    set /a FAIL+=1
)
set /a TOTAL+=1

:: Test 2: Null to non-optional (should fail)
echo [2] Testing: Null rejected in non-optional
mkdir %TEMP%\test2_proj\src\app 2>nul
echo name=t2 > %TEMP%\test2_proj\zenith.ztproj
echo version=0.1.0 >> %TEMP%\test2_proj\zenith.ztproj
echo source_root=src >> %TEMP%\test2_proj\zenith.ztproj
echo namespace app > %TEMP%\test2_proj\src\app\main.zt
echo func demo() -^> int >> %TEMP%\test2_proj\src\app\main.zt
echo     return null >> %TEMP%\test2_proj\src\app\main.zt
echo end >> %TEMP%\test2_proj\src\app\main.zt
zt.exe check %TEMP%\test2_proj >nul 2>&1
if %errorlevel% neq 0 (
    echo    PASS ^(^expected fail^)
    set /a PASS+=1
) else (
    echo    FAIL
    set /a FAIL+=1
)
set /a TOTAL+=1

:: Test 3: Type mismatch (should fail)
echo [3] Testing: Type mismatch detected
mkdir %TEMP%\test3_proj\src\app 2>nul
echo name=t3 > %TEMP%\test3_proj\zenith.ztproj
echo version=0.1.0 >> %TEMP%\test3_proj\zenith.ztproj
echo source_root=src >> %TEMP%\test3_proj\zenith.ztproj
echo namespace app > %TEMP%\test3_proj\src\app\main.zt
echo func demo() -^> int >> %TEMP%\test3_proj\src\app\main.zt
echo     return "texto" >> %TEMP%\test3_proj\src\app\main.zt
echo end >> %TEMP%\test3_proj\src\app\main.zt
zt.exe check %TEMP%\test3_proj >nul 2>&1
if %errorlevel% neq 0 (
    echo    PASS ^(^expected fail^)
    set /a PASS+=1
) else (
    echo    FAIL
    set /a FAIL+=1
)
set /a TOTAL+=1

:: Test 4: Unresolved name (should fail)
echo [4] Testing: Unresolved name detected
mkdir %TEMP%\test4_proj\src\app 2>nul
echo name=t4 > %TEMP%\test4_proj\zenith.ztproj
echo version=0.1.0 >> %TEMP%\test4_proj\zenith.ztproj
echo source_root=src >> %TEMP%\test4_proj\zenith.ztproj
echo namespace app > %TEMP%\test4_proj\src\app\main.zt
echo func demo() -^> int >> %TEMP%\test4_proj\src\app\main.zt
echo     return undefined_var >> %TEMP%\test4_proj\src\app\main.zt
echo end >> %TEMP%\test4_proj\src\app\main.zt
zt.exe check %TEMP%\test4_proj >nul 2>&1
if %errorlevel% neq 0 (
    echo    PASS ^(^expected fail^)
    set /a PASS+=1
) else (
    echo    FAIL
    set /a FAIL+=1
)
set /a TOTAL+=1

echo.
echo ================================
echo Results:
echo ================================
echo Total:   %TOTAL%
echo Passed:  %PASS%
echo Failed:  %FAIL%
echo.

if %FAIL% equ 0 (
    echo ALL TESTS PASSED!
) else (
    echo SOME TESTS FAILED
)
