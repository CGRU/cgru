if exist ..\override.cmd call ..\override.cmd

PUSHD ..\..
call setup.cmd
POPD

python render.py %*
