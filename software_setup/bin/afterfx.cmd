@echo off

call %CGRU_LOCATION%\software_setup\setup_afterfx.cmd

start "AfterFX" "%APP_DIR%\AfterFX.exe" %*
