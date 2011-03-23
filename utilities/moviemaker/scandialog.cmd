pushd ..\..
call setup.cmd
popd

if exist override.cmd call override.cmd

python scandialog.py %*
