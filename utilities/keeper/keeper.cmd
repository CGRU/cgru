@echo off

set CGRU_KEEPER=%CD%
pushd ..\..
if exist setup.cmd call setup.cmd
popd

set pyver=3.2.2
set PYTHONPATH=%AF_ROOT%\bin_pyaf\%pyver%;%PYTHONPATH%
set python=%CGRU_KEEPER%\Python
if not exist %python% set python=%CGRU_LOCATION%\utilities\python\%pyver%

if [%1] == [] (
   start %python%\pythonw keeper.py
) else (
   %python%\python keeper.py
)
