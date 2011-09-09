@echo off

set CGRU_KEEPER=%CD%
pushd ..\..
if exist setup.cmd call setup.cmd
popd

set PYTHONPATH=%AF_ROOT%\bin_pyaf\3.2.2;%PYTHONPATH%

set python=%CGRU_KEEPER%\Python

if [%1] == [] (
   start %python%\pythonw keeper.py
) else (
   %python%\python keeper.py
)
