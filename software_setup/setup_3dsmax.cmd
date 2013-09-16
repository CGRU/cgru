rem Source general for all soft directives:
call %CGRU_LOCATION%\software_setup\setup__all.cmd

For /F "Tokens=*" %%I in ('dir /b "C:\Program Files\Autodesk\3ds Max*"') Do set "MAX_LOCATION=C:\Program Files\Autodesk\%%I"

set "PATH=%CGRU_LOCATION%\plugins\max;%PATH%"

set APP_DIR=%MAX_LOCATION%
set APP_EXE=3dsmax.exe

rem Define location:
set locate_file=%CGRU_LOCATION%\software_setup\locate_3dsmax.cmd
if exist %locate_file% call %locate_file%

set MAX_LOCATION=%APP_DIR%
