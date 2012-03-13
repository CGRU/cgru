PUSHD ..\..
call setup.cmd
POPD

set BLENDER_LOCATION=c:\cg\soft\Blender

if exist override.cmd call override.cmd

%BLENDER_LOCATION%\blender.exe %*
