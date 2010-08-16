call setup_afanasy.cmd

SET NUKE_AF_VERSION=python

SET NUKE_AF_PATH=%AF_ROOT%\plugins\nuke\%NUKE_AF_VERSION%

SET NUKE_PATH=%NUKE_AF_PATH%

SET NUKE_EXEC=C":\Program Files\Nuke6.0v5\Nuke6.0.exe"

IF EXIST override.cmd CALL override.cmd
