rem Setup Afanasy:
PUSHD ..\..
call setup.cmd
POPD

rem Set Maya location:
set MAYA_LOCATION=c:\cg\soft\Maya2009-x64

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
