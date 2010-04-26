set MXX_RU_CPP_TOOLSET=vc9
set VSINSTALLDIR="C:\Program Files (x86)\Microsoft Visual Studio 9.0"

set MAYA_VERSION=2011
set MAYA_ARCH=-x64
call %VSINSTALLDIR%\VC\vcvarsall.bat amd64

set MAYA_LOCATION="C:\Program Files\Autodesk\Maya%MAYA_VERSION%"

echo Building CGRU for Maya %MAYA_VERSION%%MAYA_ARCH% ...
ruby cgru_plugin.mxw.rb %*

pause
