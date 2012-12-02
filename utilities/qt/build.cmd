set VSINSTALLDIR="C:\Program Files (x86)\Microsoft Visual Studio 10.0"
set VCVARS=amd64
set qt=qt-everywhere-opensource-src-4.8.4

if exist override.cmd call override.cmd

call %VSINSTALLDIR%\VC\vcvarsall.bat %VCVARS%

cd %qt%

configure.exe -platform win32-msvc2010 -release -opensource -static -no-scripttools -no-multimedia -no-script -no-webkit -nomake examples -nomake demos -no-gif
nmake

pause
