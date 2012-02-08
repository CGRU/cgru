@echo off

rem Source general for all soft directives:
call %CGRU_LOCATION%\software_setup\setup__all.cmd


REM Locate C4D:
set C4F_LOCATION=C:\Program Files\MAXON\CINEMA 4D R13
set C4D_EXEC=CINEMA 4D 64 Bit.exe
set C4D_USER_FOLDER=%USERPROFILE%\AppData\Roaming\MAXON\CINEMA 4D R13_05DFD2A0

REM Here additional folders can get specified where plugins should get copied from
REM set C4D_ADDITIONAL_PLUGIN_FOLDERS="PATH2;PATH2"


REM CGRU for C4D add-ons location, override it, or simple launch from current folder as an example
set C4D_CGRU_LOCATION=%CGRU_LOCATION%\plugins\c4d
set C4D_CGRU_SCRIPTS_LOCATION=%C4D_CGRU_LOCATION%\scripts

set C4D_AF_SCRIPTS_LOCATION=%AF_ROOT%\plugins\c4d

REM Define that the c4d-render-script can get found
set C4D_RENDER_SCRIPT="%C4D_CGRU_LOCATION%\render.py"

REM Locate:
set locate_file=%CGRU_LOCATION%\software_setup\locate_c4d.cmd
if exist %locate_file% call %locate_file%

set C4D_PLUGIN_LOCATION=%C4D_USER_FOLDER%\plugins
set C4D_PREFERENCES_LOCATION=%C4D_USER_FOLDER%\prefs

REM Copy the python_init.py file to the user
IF EXIST "%C4D_PREFERENCES_LOCATION%\python" (
    copy /Y "%C4D_CGRU_LOCATION%\python_init.py" "%C4D_PREFERENCES_LOCATION%\python\"
    echo "python_init.py got Copied"
) ELSE (
    echo "ERROR: The User-Directory did not exist so the current python_init.py could NOT get copied!!!"
    echo "       Please set C4D_USER_FOLDER to point to the User-Directory!"
)



set C4D_EXEC=%C4F_LOCATION%\%C4D_EXEC%
echo "C4D_EXEC: %C4D_EXEC%"

set APP_DIR=%C4F_LOCATION%
set APP_EXE=%C4D_EXEC%