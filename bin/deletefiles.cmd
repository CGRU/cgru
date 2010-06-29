@echo off

if defined CGRU_LOCATION (
   python %CGRU_LOCATION%\bin\deletefiles.py %*
) else (
   python %0\..\deletefiles.py %*
)
