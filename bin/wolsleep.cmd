@echo off

if defined CGRU_LOCATION (
   python %CGRU_LOCATION%\bin\wolsleep.py %*
) else (
   python %0\..\wolsleep.py %*
)
