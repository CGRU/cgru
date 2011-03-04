echo off
SET MSVCPATH="C:\Program Files (x86)\Microsoft Visual Studio 9.0\VC"

pushd ..\..\..\..
set cgru=%CD%
popd

SET QTS=%cgru%\utilities\qt\qt-everywhere-opensource-src-4.7.1-static
SET PYTHONS=%cgru%\utilities\python

SET CGRU_PYTHON=%PYTHONS%\2.7.1

if exist override.cmd call override.cmd

if exist %CGRU_PYTHON% SET "PATH=%CGRU_PYTHON%;%PATH%"

rem Get Afanasy sources revision:
pushd ..
set folder=%CD%
cd %cgru%\utilities
call getrevision.cmd %folder%
popd

python build.py

pause
