@echo off

call %CGRU_LOCATION%\software_setup\setup_houdini.cmd

"%APP_DIR%\bin\hython2.7.exe" %HOUDINI_CGRU_PATH%\hrender_af.py %*
