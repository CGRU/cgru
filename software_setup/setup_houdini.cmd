rem Source general for all soft directives:
call %CGRU_LOCATION%\software_setup\setup__all.cmd

IF NOT DEFINED HOUDINI_LOCATION (
    For /F "Tokens=*" %%I in ('dir /b "C:\Program Files\Side Effects Software\Houdini 1*"') Do set "HOUDINI_LOCATION=C:\Program Files\Side Effects Software\%%I"
) ELSE (
    echo "HOUDINI_LOCATION already set: %HOUDINI_LOCATION%"
)

rem Define location:
set "locate_file=%CGRU_LOCATION%\software_setup\locate_houdini.cmd"
if exist %locate_file% call %locate_file%

set APP_DIR=%HOUDINI_LOCATION%
set APP_EXE=%HOUDINI_LOCATION%\bin\houdini.exe

rem Define location:
set locate_file=%CGRU_LOCATION%\software_setup\locate_houdini.cmd
if exist %locate_file% call %locate_file%

set HOUDINI_LOCATION=%APP_DIR%

set HOUDINI_CGRU_PATH=%CGRU_LOCATION%\plugins\houdini

IF DEFINED HOUDINI_PATH (
    set "HOUDINI_PATH=%HOUDINI_CGRU_PATH%;%HOUDINI_PATH%;&"
) ELSE (
    set "HOUDINI_PATH=%HOUDINI_CGRU_PATH%;&"
)

set PYTHONPATH=%HOUDINI_CGRU_PATH%;%PYTHONPATH%
