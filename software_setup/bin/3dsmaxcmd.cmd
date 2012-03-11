@echo off

call %CGRU_LOCATION%\software_setup\setup_3dsmax.cmd

"%APP_DIR%\3dsmaxcmd.exe" %*
