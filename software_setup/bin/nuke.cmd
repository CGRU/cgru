@echo off

call %CGRU_LOCATION%\software_setup\setup_nuke.cmd

if not exist "%APP_EXE%" echo Can't find nuke %APP_EXE%

"%APP_EXE%" %*
