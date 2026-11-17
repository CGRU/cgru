@echo off
setlocal EnableExtensions

rem Update this CGRU install from the latest GitHub Windows release.
rem 1) Stops Keeper / Afanasy processes and services from this folder (remembers them)
rem 2) Mirrors the zip into this folder (adds/updates AND deletes files missing in release)
rem 3) Restarts what was running
rem
rem Kept locally even if absent from zip:
rem   update_from_github.cmd / .ps1, software_setup\locate_*, .git\
rem
rem Run from the CGRU root (folder with version.txt / setup.cmd).
rem
rem Optional env:
rem   CGRU_GITHUB_REPO  owner/name  (default: alexantoshuk/cgru)
rem   CGRU_UPDATE_DEST   target dir  (default: current directory)

set "SCRIPT_DIR=%~dp0"
set "DEST=%CGRU_UPDATE_DEST%"
if "%DEST%"=="" set "DEST=%CD%"

if not exist "%DEST%\version.txt" if not exist "%DEST%\setup.cmd" (
    echo ERROR: "%DEST%" does not look like a CGRU root ^(no version.txt / setup.cmd^).
    echo cd into the unpacked release folder and run again.
    exit /b 1
)

powershell -NoProfile -ExecutionPolicy Bypass -File "%SCRIPT_DIR%update_from_github.ps1" -Dest "%DEST%"
set "ERR=%ERRORLEVEL%"
if not "%ERR%"=="0" (
    echo.
    echo Update failed.
    exit /b %ERR%
)
exit /b 0
