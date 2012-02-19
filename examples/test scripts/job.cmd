PUSHD ..\..
call setup.cmd
POPD

python job.py %*
