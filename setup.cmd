@echo off

rem Set CGRU root:
SET CGRU_LOCATION=%CD%

rem Add CGRU bin to path:
SET "PATH=%CGRU_LOCATION%\bin;%PATH%"

rem Add software to PATH:
SET "PATH=%CGRU_LOCATION%\software_setup\bin;%PATH%"

rem Python module path:
SET CGRU_PYTHON=%CGRU_LOCATION%\lib\python
if defined PYTHONPATH (
   SET PYTHONPATH=%CGRU_PYTHON%;%PYTHONPATH%
) else (
   SET PYTHONPATH=%CGRU_PYTHON%
)

rem Get CGRU version:
For /F "Tokens=*" %%I in ('type version.txt') Do Set CGRU_VERSION=%%I
echo CGRU_VERSION %CGRU_VERSION%

rem Call custom setup scripts:
For /F "Tokens=*" %%I in ('dir /b setup_*.cmd') Do call %%I
