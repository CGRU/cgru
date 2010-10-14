SET QTS=..\project.qmake\qt-everywhere-opensource-src-4.7.0-static
SET MSVCPATH="C:\Program Files (x86)\Microsoft Visual Studio 9.0\VC"
SET RUBYDIR=c:\ruby
SET PYTHONS=C:\cg\tools\python
SET python=python

if exist override.cmd call override.cmd
if exist %QTDIR%\bin\qtvars.bat call %QTDIR%\bin\qtvars.bat

SET PATH=%RUBYDIR%\bin;%PATH%

%python% build.py

pause
