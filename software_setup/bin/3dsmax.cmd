@echo off

call %CGRU_LOCATION%\software_setup\setup_3dsmax.cmd

start "%APP_DIR%\3dsmax.exe" %*
