@echo off

call %CGRU_LOCATION%\software_setup\setup_blender.cmd

if [%1] == [] (
   start "Blender" "%APP_EXE%" %*
) else (
"%APP_EXE%" %*
)
