@echo off

call %CGRU_LOCATION%\software_setup\setup_maya.cmd

start "MayaPY" "%APP_DIR%\bin\mayapy.exe" %*
