@echo off

PUSHD %0\..\..
call setup.cmd
POPD

start %AF_ROOT%\bin\aftalk.exe %*
