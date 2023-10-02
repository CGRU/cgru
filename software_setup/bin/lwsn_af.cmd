@echo off

if not exist %CGRU_LOCATION% echo %CGRU_LOCATION% does not exist!
call %CGRU_LOCATION%\software_setup\setup_lightwave.cmd

if not exist %APP_DIR% echo %APP_DIR% does not exist!
@REM Expects arguments of the form {lwsn_path} -3 -c"{config_dir}" -d"{content_dir}" "{scene_path}"
"%APP_DIR%\bin\lwsn.exe" %*
