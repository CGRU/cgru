@echo off

SET AF_ROOT=%CD%
SET AF_PYTHON=%AF_ROOT%\python
SET PYTHONPATH=%AF_PYTHON%;%PYTHONPATH%

rem SET AF_USERNAME=%USERNAME%
rem SET AF_HOSTNAME=%COMPUTERNAME%

SET "PATH=%AF_ROOT%\bin;%PATH%"

rem Call custom setup scripts:
For /F "Tokens=*" %%I in ('dir /b setup_*.cmd') Do call %%I
