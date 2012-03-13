if exist override.cmd call override.cmd

rem Cmake finds 'Qt' by searching for 'qmake' in 'PATH' at first:
SET PATH=C:\Qt\4.6.2-vc2008\bin;%PATH%
rem Specify exact python:
rem SET AF_PYTHON_INCLUDE_PATH=C:\Python31\include
rem SET AF_PYTHON_LIBRARIES=C:\Python31\libs\libpython31.a

cmake .

pause
