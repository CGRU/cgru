@echo off

rem Source general for all soft directives:
call "%CGRU_LOCATION%\software_setup\setup__all.cmd"

rem Search for Clarisse installation:
set "ISOTROPIX=%SystemDrive%\Program Files\Isotropix"
For /F "Tokens=*" %%I in ('dir /b "%ISOTROPIX%\Clarisse*"') Do set "CLARISSE_LOCATION=%%I"
set "CLARISSE_LOCATION=%ISOTROPIX%\%CLARISSE_LOCATION%\Clarisse"

rem For export:
set "APP_DIR=%CLARISSE_LOCATION%"
set "APP_EXE=%APP_DIR%\clarisse.exe"

rem Customize location:
set "locate_file=%CGRU_LOCATION%\software_setup\locate_clarisse.cmd"
if exist "%locate_file%" call "%locate_file%"

echo CLARISSE="%APP_DIR%"
