set VSINSTALLDIR="C:\Program Files (x86)\Microsoft Visual Studio 9.0"
set VCVARS=x86

call %VSINSTALLDIR%\VC\vcvarsall.bat %VCVARS%

cd qt-everywhere-opensource-src-4.7.0-static-32

rem configure.exe -help
rem nmake confclean

configure.exe -platform win32-msvc2008 -release -opensource -static -no-scripttools -no-multimedia -no-script -no-webkit -nomake examples -nomake demos -no-sql-sqlite -no-gif
nmake

pause
