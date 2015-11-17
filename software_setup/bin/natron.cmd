@echo off

call %CGRU_LOCATION%\software_setup\setup_natron.cmd

if not exist "%APP_EXE%" echo Can't find Narton %APP_EXE%

"%APP_EXE%" %*
