PUSHD ..\..
call setup.cmd
POPD

set NUKE_AF_VERSION=tclcmd

SET NUKE_AF_PATH=%AF_ROOT%\plugins\nuke\%NUKE_AF_VERSION%

SET NUKE_PATH=%NUKE_AF_PATH%

SET NUKE_EXEC="C:\Program Files\Nuke6.0v5\Nuke6.0.exe"

if exist override.cmd call override.cmd
