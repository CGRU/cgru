@echo off

PUSHD ..
call setup.cmd
POPD

%AF_ROOT%\bin\afrender.exe %*
