@echo off

call %CGRU_LOCATION%\software_setup\setup_houdini.cmd

"%APP_DIR%\bin\mantra.exe" %*
