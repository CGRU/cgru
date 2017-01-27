rem @echo off
set CMAKE_GENERATOR=Visual Studio 14 2015 Win64

pushd ..\..\..
call setup.cmd
popd

setlocal EnableDelayedExpansion

rem Cmake finds 'Qt' by searching for 'qmake' in 'PATH'
SET "cgru_qt_install_dir=C:\Qt"
For /F "Tokens=*" %%I in ('dir /b "%cgru_qt_install_dir%\Qt5.8*"') Do SET "cgru_qt=%%I"
if defined cgru_qt (
	SET "cgru_qt=!cgru_qt_install_dir!\!cgru_qt!\5.8\msvc2015_64"
	echo Adding "!cgru_qt!" to PATH
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
