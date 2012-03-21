pushd ..\..
call setup.cmd
popd

if not exist render mkdir render

python render.py

pause
