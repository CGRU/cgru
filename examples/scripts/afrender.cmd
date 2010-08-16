PUSHD ..\..
call setup.cmd
POPD

call setup.cmd

PUSHD %CGRU_LOCATION%
call setup.cmd
POPD

%AF_ROOT%\bin\afrender.exe %*
