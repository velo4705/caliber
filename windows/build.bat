@echo off
setlocal EnableDelayedExpansion

:: ============================================================
::  Caliber — Windows Build Script
::  Usage: build.bat [Qt6 root]  e.g.  build.bat C:\Qt\6.7.0\msvc2019_64
::  If no argument given, Qt6_DIR must already be in PATH.
:: ============================================================

set "SCRIPT_DIR=%~dp0"
set "REPO_DIR=%SCRIPT_DIR%.."
set "DEPLOY_DIR=%SCRIPT_DIR%deploy"
set "BUILD_DIR=%REPO_DIR%\build_win"

:: Optional: accept Qt root as first argument
if not "%~1"=="" (
    set "QT_ROOT=%~1"
    set "PATH=%QT_ROOT%\bin;%PATH%"
    set "Qt6_DIR=%QT_ROOT%\lib\cmake\Qt6"
)

:: ── Verify tools ─────────────────────────────────────────────────────────────
where cmake >nul 2>&1 || (echo [ERROR] cmake not found. Install CMake and add to PATH. & exit /b 1)
where windeployqt >nul 2>&1 || (echo [ERROR] windeployqt not found. Add Qt bin dir to PATH. & exit /b 1)
where makensis >nul 2>&1 || (echo [WARN]  makensis not found. Installer will not be built.)

:: ── Configure ────────────────────────────────────────────────────────────────
echo.
echo [1/4] Configuring CMake (Release)...
cmake -S "%REPO_DIR%" -B "%BUILD_DIR%" ^
      -DCMAKE_BUILD_TYPE=Release ^
      -DCMAKE_PREFIX_PATH="%QT_ROOT%" ^
      -G "Visual Studio 17 2022" -A x64
if errorlevel 1 ( echo [ERROR] CMake configure failed. & exit /b 1 )

:: ── Build ─────────────────────────────────────────────────────────────────────
echo.
echo [2/4] Building...
cmake --build "%BUILD_DIR%" --config Release --parallel
if errorlevel 1 ( echo [ERROR] Build failed. & exit /b 1 )

:: ── Deploy ───────────────────────────────────────────────────────────────────
echo.
echo [3/4] Deploying Qt dependencies...
if exist "%DEPLOY_DIR%" rmdir /s /q "%DEPLOY_DIR%"
mkdir "%DEPLOY_DIR%"

copy "%BUILD_DIR%\Release\caliber.exe" "%DEPLOY_DIR%\" >nul
copy "%SCRIPT_DIR%caliber.ico"         "%DEPLOY_DIR%\" >nul 2>&1

windeployqt ^
    --no-translations ^
    --no-system-d3d-compiler ^
    --no-opengl-sw ^
    --dir "%DEPLOY_DIR%" ^
    "%DEPLOY_DIR%\caliber.exe"
if errorlevel 1 ( echo [ERROR] windeployqt failed. & exit /b 1 )

:: ── NSIS Installer ───────────────────────────────────────────────────────────
echo.
echo [4/4] Building NSIS installer...
where makensis >nul 2>&1
if errorlevel 1 (
    echo [SKIP] makensis not found — skipping installer.
    echo        Deploy folder ready at: %DEPLOY_DIR%
    goto :done
)

makensis "%SCRIPT_DIR%installer.nsi"
if errorlevel 1 ( echo [ERROR] NSIS build failed. & exit /b 1 )
echo.
echo [OK] Installer built: %SCRIPT_DIR%Caliber-1.1.0-Setup.exe

:done
echo.
echo Done. Deploy folder: %DEPLOY_DIR%
endlocal
