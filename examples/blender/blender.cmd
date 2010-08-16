PUSHD ..\..
call setup.cmd
POPD

set BLENDER_LOCATION="C:\Program Files\Blender Foundation\Blender"

if exist override.cmd call override.cmd

%BLENDER_LOCATION%\blender.exe %*
