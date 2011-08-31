@echo off

set CGRU_KEEPER=%CD%
pushd ..\..
if exist setup.cmd call setup.cmd
popd
set PYTHON=%CGRU_KEEPER%\Python3

rem start %PYTHON%\pythonw keeper.py
%PYTHON%\python keeper.py
