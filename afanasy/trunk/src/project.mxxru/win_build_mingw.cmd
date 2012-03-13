SET MXX_RU_CPP_TOOLSET=gcc_mingw
SET QTDIR=C:\Qt\4.6.2-mingw

SET RUBYDIR=c:\ruby
SET PYTHONLIB=python25
SET PYTHONDIR=C:\%PYTHONLIB%

if exist overrides.cmd call overrides.cmd
if exist %QTDIR%\bin\qtvars.bat call %QTDIR%\bin\qtvars.bat

SET PATH=%RUBYDIR%\bin;%PATH%

ruby afanasy.mxw.rb

pause
