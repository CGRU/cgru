@echo off

call %CGRU_LOCATION%\software_setup\setup_softimage.cmd

rem start "SoftImage" "%APP_DIR%\bin\XSI.bat" %*
"%APP_DIR%\bin\XSI.bat" %*
