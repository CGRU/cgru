rem @echo off
set CMAKE_GENERATOR=Visual Studio 9 2008 Win64

pushd ..\..\..\..
set cgru=%CD%
popd

rem Cmake finds 'Qt' by searching for 'qmake' in 'PATH'
set cgru_qt=%cgru%\utilities\qt\qt-everywhere-opensource-src-4.7.0-shared
if exist %cgru_qt% SET PATH=%cgru_qt%\bin;%PATH%

rem Specify Python:
set cgru_python=%cgru%\utilities\python\2.5.4
if exist %cgru_python% (
   echo Using CGRU Python: %cgru_python%
   SET AF_PYTHON_INCLUDE_PATH=%cgru_python%\include
   SET AF_PYTHON_LIBRARIES=%cgru_python%\libs\python25.lib
)

if exist override.cmd call override.cmd

rem Get Afanasy sources revision:
pushd ..
set folder=%CD%
cd %cgru%\utilities
call getrevision.cmd %folder%
popd

cmake -G "%CMAKE_GENERATOR%" .

cmake .

pause
