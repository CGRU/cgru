@echo off

call %CGRU_LOCATION%\software_setup\setup_fusion.cmd

if not exist "%APP_EXE%" echo Can't find Fusion %APP_EXE%

"%APP_EXE%" %*
