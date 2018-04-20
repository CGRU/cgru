rem @echo off
set CMAKE_GENERATOR=Visual Studio 15 2017 Win64

pushd ..\..\..
call setup.cmd
popd

setlocal EnableDelayedExpansion

rem Cmake finds 'Qt' by searching for 'qmake' in 'PATH'
SET "cgru_qt=C:\Qt\Qt5.10.1\5.10.1\msvc2017_64"
if exist !cgru_qt! (
	echo Adding "!cgru_qt!\bin" to PATH
	SET "PATH=!cgru_qt!\bin;%PATH%"
)
set "AF_GUI=YES"
set "AF_QT_VER=5"

rem Specify Python:
if exist %CGRU_LOCATION%\python-devel set CGRU_PYTHONDIR=%CGRU_LOCATION%\python-devel
if defined CGRU_PYTHONDIR (
   echo Building with CGRU Python: "!CGRU_PYTHONDIR!"
   SET "AF_PYTHON_INCLUDE_PATH=!CGRU_PYTHONDIR!\include"
   SET "AF_PYTHON_LIBRARIES=!CGRU_PYTHONDIR!\libs\python34.lib"
)

rem Boost:
set "cgru_boost_install_dir=C:\local"
For /F "Tokens=*" %%I in ('dir /b "%cgru_boost_install_dir%\boost*"') Do set "BOOST_ROOT=%%I"
if defined BOOST_ROOT (
	set "BOOST_ROOT=!cgru_boost_install_dir!\!BOOST_ROOT!"
	echo BOOST_ROOT set to "!BOOST_ROOT!"
)

rem Local overrides:
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
