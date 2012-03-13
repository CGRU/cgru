PUSHD ..\..
call setup.cmd
POPD

SET NUKE_CGRU_VERSION=2.7

SET NUKE_CGRU_PATH=%AF_ROOT%\plugins\nuke\%NUKE_CGRU_VERSION%

SET NUKE_PATH=%NUKE_CGRU_PATH%

SET NUKE_EXEC=c:\cg\soft\Nuke5.2v2\Nuke5.2.exe

IF EXIST override.cmd CALL override.cmd
