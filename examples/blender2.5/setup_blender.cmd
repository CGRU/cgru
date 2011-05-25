PUSHD ..
call setup.cmd
POPD

set BLENDER_LOCATION="C:\Program Files\Blender Foundation\Blender"

call %AF_ROOT%\py3k_setup.cmd 3.2

if exist override.cmd call override.cmd
