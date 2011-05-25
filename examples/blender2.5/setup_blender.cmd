PUSHD ..
call setup.cmd
POPD

set BLENDER_LOCATION="C:\Program Files\Blender Foundation\Blender"

call %AF_ROOT%\py3k_setup.cmd 3.2-utf32

rem set BLENDER_USER_SCRIPTS=%AF_ROOT%/plugins/blender2.5
rem set BLENDER_SYSTEM_SCRIPTS=%AF_ROOT%/plugins/blender2.5

rem set AF_CMD_PREFIX=.\

if exist override.cmd call override.cmd
