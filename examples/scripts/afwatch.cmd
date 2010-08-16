PUSHD ..\..
call setup.cmd
POPD

call setup.cmd

PUSHD %CGRU_LOCATION%
call setup.cmd
POPD

start %AF_ROOT%\bin\afwatch.exe %*
