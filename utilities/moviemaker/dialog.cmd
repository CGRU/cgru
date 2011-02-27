pushd ..\..
call setup.cmd
rem cd afanasy
rem if exist trunk cd trunk
rem call setup.cmd
popd

set pyver=2.7.1
set pyaf=%AF_ROOT%\bin_pyaf\%pyver%
if exist %pyaf% set PYTHONPATH=%pyaf%;%PYTHONPATH%

if exist override.cmd call override.cmd

python dialog.py %*
