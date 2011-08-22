rem @echo off
set CMAKE_GENERATOR=Visual Studio 9 2008 Win64

pushd ..\..
set cgru=%CD%
popd

rem Cmake finds 'Qt' by searching for 'qmake' in 'PATH'
set cgru_qt=%cgru%\utilities\qt\qt-everywhere-opensource-src-4.7.3
if exist %cgru_qt% SET "PATH=%cgru_qt%\bin;%PATH%"

if exist override.cmd call override.cmd

set folder=src
cmake -G "%CMAKE_GENERATOR%" %folder%
cmake %folder%

pause
