@echo off

call %CGRU_LOCATION%\software_setup\setup_maya.cmd

"%APP_DIR%\bin\Render.exe" %*
