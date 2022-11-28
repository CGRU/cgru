rem Source general for all soft directives:
call %CGRU_LOCATION%\software_setup\setup__all.cmd

IF NOT DEFINED AE_LOCATION (
    For /F "Tokens=*" %%I in ('dir /b "C:\Program Files\Adobe\Adobe After Effects*"') Do set "AE_LOCATION=C:\Program Files\Adobe\%%I\Support Files"
) ELSE (
    echo "AE_LOCATION already set: %AE_LOCATION%"
)

set APP_DIR=%AE_LOCATION%
set APP_EXE=%AE_LOCATION%\aerender.exe

rem Define location:
set locate_file=%CGRU_LOCATION%\software_setup\locate_afterfx.cmd
if exist %locate_file% call %locate_file%

set AE_LOCATION=%APP_DIR%
