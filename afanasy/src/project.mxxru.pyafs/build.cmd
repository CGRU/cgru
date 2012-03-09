echo off
SET MSVCPATH="C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC"

pushd ..\..\..
call setup.cmd
popd

SET PYTHONS=%CGRU_LOCATION%\utilities\python

if exist override.cmd call override.cmd

rem Get Afanasy sources revision:
pushd ..
set folder=%CD%
cd %CGRU_LOCATION%\utilities
call getrevision.cmd %folder%
popd

%CGRU_PYTHONEXE% build.py

pause
