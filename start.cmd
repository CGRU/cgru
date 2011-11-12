@echo off

cd "%0\.."
call setup.cmd
set keeper=%CGRU_LOCATION%\utilities\keeper\keeper.py

cd "%HOMEPATH%"

if [%1] == [] (
   set CGRU_KEEPER_CMD="%CGRU_PYTHONDIR%\pythonw.exe" "%keeper%"
   start "Keeper" "%CGRU_PYTHONDIR%\pythonw.exe" "%keeper%"
) else (
   "%CGRU_PYTHONEXE%" "%keeper%"
)
