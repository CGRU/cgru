SET MSVCPATH="C:\Program Files (x86)\Microsoft Visual Studio 9.0\VC"

pushd ..\..\..\..
set cgru=%CD%
popd

SET QTS=%cgru%\utilities\qt\qt-everywhere-opensource-src-4.7.0-static
SET PYTHONS=%cgru%\utilities\python

if exist override.cmd call override.cmd

python build.py

pause
