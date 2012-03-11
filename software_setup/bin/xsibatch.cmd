@echo off

call %CGRU_LOCATION%\software_setup\setup_softimage.cmd

"%APP_DIR%\bin\XSIBatch.bat" %*
