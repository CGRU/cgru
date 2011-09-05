@echo off

set CGRU_KEEPER=%CD%
pushd ..\..
if exist setup.cmd call setup.cmd
popd

call %AF_ROOT%\py3k_setup.cmd 3.2.1

set PYTHON=%CGRU_KEEPER%\Python3

rem start %PYTHON%\pythonw keeper.py
%PYTHON%\python keeper.py
