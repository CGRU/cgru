@echo off

set CGRU_KEEPER=%CD%
pushd ..\..
if exist setup.cmd call setup.cmd
popd

call %AF_ROOT%\py3k_setup.cmd 3.2.2

set PYTHON=%CGRU_KEEPER%\Python

if [%1] == [] (
   start %PYTHON%\pythonw keeper.py
) else (
   %PYTHON%\python keeper.py
)
