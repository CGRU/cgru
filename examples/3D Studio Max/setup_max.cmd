pushd ..
call setup.cmd
popd

set MAX_LOCATION=C:\Program Files\Autodesk\3ds Max 2011

set PATH=%CGRU_LOCATION%\plugins\max;%PATH%

if exist override.cmd call override.cmd

set PATH=%MAX_LOCATION%;%PATH%
