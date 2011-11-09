@echo off

call %CGRU_LOCATION%\software_setup\setup_afterfx.cmd

"%APP_DIR%\aerender.exe" %*
