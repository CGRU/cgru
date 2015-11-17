set MXX_RU_CPP_TOOLSET=vc10
set VSINSTALLDIR="C:\Program Files (x86)\Microsoft Visual Studio 10.0"
set VCVARS=amd64

set MAYA_VERSION=2016
set MAYA_LOCATION="C:\Program Files\Autodesk\Maya%MAYA_VERSION%"

IF EXIST override.cmd CALL override.cmd

call %VSINSTALLDIR%\VC\vcvarsall.bat %VCVARS%

echo Building CGRU for Maya %MAYA_VERSION% ...
ruby cgru_plugin.mxw.rb %*

pause
