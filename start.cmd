@echo off

cd %0\..
call setup.cmd
set keeper=%CGRU_LOCATION%\utilities\keeper\keeper.py

if [%1] == [] (
   start "Keeper" "%CGRU_PYTHONDIR%\pythonw.exe" "%keeper%"
) else (
   "%CGRU_PYTHONEXE%" "%keeper%"
)
