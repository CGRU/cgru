rem @echo off
set CMAKE_GENERATOR=Visual Studio 10 Win64

pushd ..\..\..
call setup.cmd
popd

rem Cmake finds 'Qt' by searching for 'qmake' in 'PATH'
set cgru_qt=%CGRU_LOCATION%\utilities\qt\qt-everywhere-opensource-src-4.8.4
if exist %cgru_qt% SET "PATH=%cgru_qt%\bin;%PATH%"

rem Specify Python:
if defined CGRU_PYTHONDIR (
   echo Building with CGRU Python: %CGRU_PYTHONDIR%
   SET AF_PYTHON_INCLUDE_PATH=%CGRU_PYTHONDIR%\include
   SET AF_PYTHON_LIBRARIES=%CGRU_PYTHONDIR%\libs\python33.lib
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
