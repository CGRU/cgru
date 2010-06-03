rem Cmake finds 'Qt' by searching for 'qmake' in 'PATH' at first:
rem SET PATH=C:\Qt\4.6.2-vc2008\bin;%PATH%
rem Specify exact python:
rem SET AF_PYTHON_INCLUDE_PATH=C:\Python25\include
rem SET AF_PYTHON_LIBRARIES=C:\Python25\libs\python25.lib

cmake -G "Visual Studio 9 2008 Win64" .

if exist override.cmd call override.cmd

cmake .

pause
