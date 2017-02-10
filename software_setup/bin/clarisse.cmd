@echo off

call %CGRU_LOCATION%\software_setup\setup_clarisse.cmd

if [%1] == [] (
   start "Clarisse" "%APP_EXE%" %*
) else (
"%APP_EXE%" %*
)
