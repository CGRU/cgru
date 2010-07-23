SET MXX_RU_CPP_TOOLSET=vc9
SET QTDIR=C:\Qt\4.6.2-vc2008
SET MSVCPATH="C:\Program Files\Microsoft Visual Studio 9.0\VC"

SET RUBYDIR=c:\ruby
SET PYTHONLIB=python25
SET PYTHONDIR=C:\%PYTHONLIB%

if exist overrides.cmd call overrides.cmd
if exist %QTDIR%\bin\qtvars.bat call %QTDIR%\bin\qtvars.bat

call %MSVCPATH%\vcvarsall.bat x86

SET PATH=%RUBYDIR%\bin;%PATH%

ruby afanasy.mxw.rb --mxx-show-cmd

pause
