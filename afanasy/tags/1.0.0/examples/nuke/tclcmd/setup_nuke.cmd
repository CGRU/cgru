PUSHD ..\..\..
call setup.cmd
POPD

set NUKE_CGRU_VERSION=tclcmd

SET NUKE_CGRU_PATH=%AF_ROOT%\plugins\nuke\%NUKE_CGRU_VERSION%

SET NUKE_EXEC=c:\cg\soft\Nuke4.7v2\Nuke4.7.exe

if exist override.cmd call override.cmd
