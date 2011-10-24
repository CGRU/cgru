SET MXX_RU_CPP_TOOLSET=vc9
SET QTDIR=..\project.qmake\qt-everywhere-opensource-src-4.7.0-shared
SET MSVCPATH="C:\Program Files (x86)\Microsoft Visual Studio 9.0\VC"

SET PATH=%QTDIR%\bin;%PATH%

SET PYTHONLIB=python25
SET PYTHONDIR=C:\%PYTHONLIB%

if exist override.cmd call override.cmd
rem if exist %QTDIR%\bin\qtvars.bat call %QTDIR%\bin\qtvars.bat
call %QTDIR%\bin\qtvars.bat

call %MSVCPATH%\vcvarsall.bat amd64

ruby afanasy.mxw.rb %*

pause
