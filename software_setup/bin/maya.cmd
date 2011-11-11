@echo off

call %CGRU_LOCATION%\software_setup\setup_maya.cmd

start "Maya" "%APP_DIR%\bin\maya.exe" %*
