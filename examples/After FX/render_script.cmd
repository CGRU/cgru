pushd ..\..
call setup.cmd
popd

if not exist render mkdir render

"%CGRU_PYTHONEXE%" render.py

pause
