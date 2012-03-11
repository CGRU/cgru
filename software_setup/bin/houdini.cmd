@echo off

call %CGRU_LOCATION%\software_setup\setup_houdini.cmd

start "Houdini" "%APP_DIR%\bin\houdini.exe" %*
