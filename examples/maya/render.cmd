PUSHD ..\..
call setup.cmd
POPD

if not exist render mkdir render

python render.py

pause
