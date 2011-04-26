@echo off

if defined CGRU_LOCATION (
   python %CGRU_LOCATION%\bin\wolwake.py %*
) else (
   python %0\..\wolwake.py %*
)
