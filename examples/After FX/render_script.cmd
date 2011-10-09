pushd ..\..
call setup.cmd
popd

"%CGRU_PYTHONEXE%" render.py

pause
