SET QTS=C:\qt\qt-everywhere-opensource-src-4.6.3-static
SET MSVCPATH="C:\Program Files (x86)\Microsoft Visual Studio 9.0\VC"
SET RUBYDIR=c:\ruby
SET PYTHONS=C:\pythons
SET python=python

if exist overrides.cmd call overrides.cmd
if exist %QTDIR%\bin\qtvars.bat call %QTDIR%\bin\qtvars.bat

SET PATH=%RUBYDIR%\bin;%PATH%

%python% build.py

pause
