rem Source general for all soft directives:
call %CGRU_LOCATION%\software_setup\setup__all.cmd

set AE_LOCATION=C:\Program Files\Adobe\Adobe After Effects CS5

set APP_DIR=%AE_LOCATION%
set APP_EXE=%AE_LOCATION%\Support Files\aerender.exe

rem Define location:
set locate_file=%CGRU_LOCATION%\software_setup\locate_afterfx.cmd
if exist %locate_file% call %locate_file%

set AE_LOCATION=%APP_DIR%
