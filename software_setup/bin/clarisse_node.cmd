@echo off

call %CGRU_LOCATION%\software_setup\setup_clarisse.cmd

"%APP_DIR%\cnode.exe" %*
