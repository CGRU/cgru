@echo off

cd "%0\.."

For /F "Tokens=*" %%I in ('dir /b /ad') Do if exist "%%I/clear.cmd" "%%I/clear.cmd"
