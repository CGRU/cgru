rem @echo off
set CMAKE_GENERATOR=Visual Studio 14 2015 Win64

pushd ..\..\..
call setup.cmd
popd

rem Cmake finds 'Qt' by searching for 'qmake' in 'PATH'
rem set cgru_qt=%CGRU_LOCATION%\utilities\qt\qt-everywhere-opensource-src-4.8.6
set "cgru_qt=C:\Qt\Qt5.6.0\5.6\msvc2015_64"
if exist %cgru_qt% SET "PATH=%cgru_qt%\bin;%PATH%"
set "AF_GUI=YES"

rem Specify Python:
if exist %CGRU_LOCATION%\python-devel set CGRU_PYTHONDIR=%CGRU_LOCATION%\python-devel
if defined CGRU_PYTHONDIR (
   echo Building with CGRU Python: %CGRU_PYTHONDIR%
   SET AF_PYTHON_INCLUDE_PATH=%CGRU_PYTHONDIR%\include
   SET AF_PYTHON_LIBRARIES=%CGRU_PYTHONDIR%\libs\python34.lib
)

if exist override.cmd call override.cmd

rem Get Afanasy sources revision:
pushd ..
set folder=%CD%
cd %CGRU_LOCATION%\utilities
call getrevision.cmd %folder%
popd

cmake -G "%CMAKE_GENERATOR%" .

cmake .

pause
