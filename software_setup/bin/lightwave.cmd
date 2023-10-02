@echo off

call %CGRU_LOCATION%\software_setup\setup_lightwave.cmd

start "Layout" "%APP_DIR%\bin\Layout.exe" %*
