SET MXX_RU_CPP_TOOLSET=vc9
SET QTDIR=C:\cg\tools\qt\qt-everywhere-opensource-src-4.6.3-shared
SET MSVCPATH="C:\Program Files (x86)\Microsoft Visual Studio 9.0\VC"

SET PATH=%QTDIR%\bin;%PATH%

SET RUBYDIR=c:\ruby
SET PYTHONLIB=python25
SET PYTHONDIR=C:\%PYTHONLIB%

if exist overrides.cmd call overrides.cmd
if exist %QTDIR%\bin\qtvars.bat call %QTDIR%\bin\qtvars.bat

call %MSVCPATH%\vcvarsall.bat amd64

SET PATH=%RUBYDIR%\bin;%PATH%

ruby afanasy.mxw.rb %*

pause
