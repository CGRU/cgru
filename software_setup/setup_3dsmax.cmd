set MAX_LOCATION=C:\Program Files\Autodesk\3ds Max 2012

set PATH=%CGRU_LOCATION%\plugins\max;%PATH%

if exist override.cmd call override.cmd

set "PATH=%MAX_LOCATION%;%PATH%"


set APP_DIR=%MAX_LOCATION%
set APP_EXE=3dsmax.exe
