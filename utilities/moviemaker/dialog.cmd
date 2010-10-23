pushd ..\..
call setup.cmd
cd afanasy
if exist trunk cd trunk
call setup.cmd
popd

set pyver=254_32
set pyaf=%AF_ROOT%\bin_pyaf\%pyver%
if exist %pyaf% set PYTHONPATH=%pyaf%;%PYTHONPATH%

if exist override.cmd call override.cmd

python dialog.py %*
