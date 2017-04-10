PUSHD ..\..
call setup.cmd
POPD

%CGRU_PYTHONEXE% job.py %*
