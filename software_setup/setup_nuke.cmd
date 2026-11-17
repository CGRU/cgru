rem Source general for all soft directives:
call %CGRU_LOCATION%\software_setup\setup__all.cmd

rem Setup CGRU common scripts if location provided
set NUKE_CGRU_PATH=%CGRU_LOCATION%\plugins\nuke
if defined CGRU_LOCATION (
   if defined NUKE_PATH (
      set NUKE_PATH=%NUKE_CGRU_PATH%;%NUKE_PATH%
   ) else (
      set NUKE_PATH=%NUKE_CGRU_PATH%
   )
)

rem Default number of threads for rendering:
rem SET NUKE_AF_RENDERTHREADS=2

rem Nuke render launcher:
SET NUKE_AF_RENDER=nuke

rem Prefer job env NUKE_EXEC (from Nuke "Get Nuke Environment").
rem (goto used so %vars% expand correctly in cmd.exe)
if not "%NUKE_EXEC%"=="" goto :nuke_from_exec
if not "%NUKE_LOCATION%"=="" goto :nuke_located

For /F "Tokens=*" %%I in ('dir /b "C:\Program Files\Nuke*" 2^>nul') Do set "NUKE_LOCATION=C:\Program Files\%%I"

For /F "Tokens=*" %%I in ('dir /b "%NUKE_LOCATION%\Nuke*.*.exe" 2^>nul') Do set "NUKE_EXEC=%NUKE_LOCATION%\%%I"

SET NUKE_DIR=%NUKE_LOCATION%
SET APP_DIR=%NUKE_LOCATION%
SET APP_EXE=%NUKE_EXEC%

rem Locate (Keeper default) only when env did not pin a build:
set locate_file=%CGRU_LOCATION%\software_setup\locate_nuke.cmd
if exist %locate_file% call %locate_file%

SET NUKE_LOCATION=%APP_DIR%
goto :nuke_cgru

:nuke_from_exec
echo NUKE_EXEC from environment: %NUKE_EXEC%
for %%I in ("%NUKE_EXEC%") do set "NUKE_LOCATION=%%~dpI"
rem %%~dpI has trailing backslash — strip it
if "%NUKE_LOCATION:~-1%"=="\" set "NUKE_LOCATION=%NUKE_LOCATION:~0,-1%"
SET NUKE_DIR=%NUKE_LOCATION%
SET APP_DIR=%NUKE_LOCATION%
SET APP_EXE=%NUKE_EXEC%
goto :nuke_cgru

:nuke_located
echo NUKE_LOCATION from environment: %NUKE_LOCATION%
SET NUKE_DIR=%NUKE_LOCATION%
SET APP_DIR=%NUKE_LOCATION%
if not "%NUKE_EXEC%"=="" (
    SET "APP_EXE=%NUKE_EXEC%"
    goto :nuke_cgru
)
For /F "Tokens=*" %%I in ('dir /b "%NUKE_LOCATION%\Nuke*.*.exe" 2^>nul') Do set "APP_EXE=%NUKE_LOCATION%\%%I"
if "%APP_EXE%"=="" set "APP_EXE=%NUKE_LOCATION%\Nuke.exe"

:nuke_cgru
SET LM_LICENSE_FILE=%NUKE_LOCATION%\nuke.lic
SET LM_LICENSE_FILE=%NUKE_LOCATION%\foundry.lic
