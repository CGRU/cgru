if exist override.cmd call override.cmd

PUSHD ..
call setup.cmd
POPD

if not exist render mkdir render

python render.py %*

pause
