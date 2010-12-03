PUSHD ..
call setup.cmd
POPD

rem overrides (set custom values there):
if exist override.cmd call override.cmd
