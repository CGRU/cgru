@echo off
set MXX_RU_CPP_TOOLSET=vc7
set VSINSTALLDIR="C:\Program Files (x86)\Microsoft Visual Studio 8"

rem set MAYA_VERSION=6.0
rem set MAYA_ARCH=
rem call "C:\Program Files (x86)\Microsoft Visual Studio .NET 2003\VC7\bin\vcvars32.bat"
rem call :build_cgru %*

rem set MAYA_VERSION=6.5
rem set MAYA_ARCH=
rem call %VSINSTALLDIR%\VC\vcvarsall.bat x86
rem call :build_cgru %*

rem set MAYA_VERSION=7.0
rem set MAYA_ARCH=
rem call %VSINSTALLDIR%\VC\vcvarsall.bat x86
rem call :build_cgru %*

rem set MAYA_VERSION=8.0
rem set MAYA_ARCH=
rem call %VSINSTALLDIR%\VC\vcvarsall.bat x86
rem call :build_cgru %*

rem set MAYA_VERSION=8.0
rem set MAYA_ARCH=-x64
rem call %VSINSTALLDIR%\VC\vcvarsall.bat amd64
rem call :build_cgru %*

rem set MAYA_VERSION=8.5
rem set MAYA_ARCH=
rem call %VSINSTALLDIR%\VC\vcvarsall.bat x86
rem call :build_cgru %*

rem set MAYA_VERSION=8.5
rem set MAYA_ARCH=-x64
rem call %VSINSTALLDIR%\VC\vcvarsall.bat amd64
rem call :build_cgru %*

rem set MAYA_VERSION=2008
rem set MAYA_ARCH=
rem call %VSINSTALLDIR%\VC\vcvarsall.bat x86
rem call :build_cgru %*

rem set MAYA_VERSION=2008
rem set MAYA_ARCH=-x64
rem call %VSINSTALLDIR%\VC\vcvarsall.bat amd64
rem call :build_cgru %*

set MAYA_VERSION=2009
set MAYA_ARCH=
call %VSINSTALLDIR%\VC\vcvarsall.bat x86
call :build_cgru %*

set MAYA_VERSION=2009
set MAYA_ARCH=-x64
call %VSINSTALLDIR%\VC\vcvarsall.bat amd64
call :build_cgru %*

GOTO:EOF

:build_cgru
   set MAYA_LOCATION="c:\cg\soft\Maya%MAYA_VERSION%%MAYA_ARCH%"
   echo Building CGRU for Maya %MAYA_VERSION%%MAYA_ARCH% ...
   ruby cgru_plugin.mxw.rb %*
   pause
GOTO:EOF
