set MXX_RU_CPP_TOOLSET=vc9
set VSINSTALLDIR="C:\Program Files (x86)\Microsoft Visual Studio 9.0"
set VCVARS=amd64

set MAYA_VERSION=2013
set MAYA_ARCH=-x64
set MAYA_LOCATION="C:\Program Files\Autodesk\Maya%MAYA_VERSION%"

IF EXIST override.cmd CALL override.cmd

call %VSINSTALLDIR%\VC\vcvarsall.bat %VCVARS%

echo Building CGRU for Maya %MAYA_VERSION%%MAYA_ARCH% ...
ruby cgru_plugin.mxw.rb %*

pause
