@echo off

call %CGRU_LOCATION%\software_setup\setup_3dsmax.cmd

start "3D MAX" "%APP_DIR%\3dsmax.exe" %*
