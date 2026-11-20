@echo off
setlocal enabledelayedexpansion

REM ==============================================================================
REM Nuke Plugin Build & Environment Manager Launcher
REM ==============================================================================
REM Dynamically determines workspace root relative to this batch file (%~dp0).
REM Never uses hardcoded absolute paths.
REM Launches PowerShell with ExecutionPolicy Bypass without requiring admin rights.
REM ==============================================================================

set "SCRIPT_DIR=%~dp0"
REM Strip trailing backslash
if "%SCRIPT_DIR:~-1%"=="\" set "SCRIPT_DIR=%SCRIPT_DIR:~0,-1%"

cd /d "%SCRIPT_DIR%"
title Nuke Plugin Build Manager

where pwsh.exe >nul 2>&1
if %ERRORLEVEL% equ 0 (
    set "PS_EXE=pwsh.exe"
) else (
    set "PS_EXE=powershell.exe"
)

"%PS_EXE%" -NoProfile -ExecutionPolicy Bypass -File "%SCRIPT_DIR%\build.ps1" -WorkspaceRoot "%SCRIPT_DIR%" %*
set "EXIT_CODE=%ERRORLEVEL%"

exit /b %EXIT_CODE%
