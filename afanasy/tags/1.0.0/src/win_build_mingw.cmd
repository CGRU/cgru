if exist override.cmd call override.cmd

SET tmpdir=tmp
if exist %tmpdir% rmdir /s /q %tmpdir%
mkdir %tmpdir%
cd %tmpdir%

cmake -G "MinGW Makefiles" -D CMAKE_INSTALL_PREFIX:PATH=../../../afanasy ..

c:\mingw\bin\mingw32-make.exe

c:\mingw\bin\mingw32-make.exe install

pause
