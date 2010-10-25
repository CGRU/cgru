rem Setup Afanasy:
PUSHD ..
call setup.cmd
POPD

set PYTHONHOME=

rem Set Maya location:
set MAYA_LOCATION=C:\Program Files\Autodesk\Maya2011

rem Set custom values if needed here:
IF EXIST override.cmd CALL override.cmd

rem Add Afanasy scripts:
set MAYA_SCRIPT_PATH=%AF_ROOT%\plugins\maya;%MAYA_SCRIPT_PATH%

set PATH=%MAYA_LOCATION%\bin;%PATH%

IF [%1]==[] (
set TOSTART=start
) ELSE (
set TOSTART=
)
