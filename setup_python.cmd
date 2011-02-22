@echo off

rem   Try to setup CGRU python, if it exists:
rem   (the same version as Afanasy compiled)

set CGRU_PYTHON=%CGRU_LOCATION%\utilities\python\2.7.1

if exist %CGRU_PYTHON% (
   echo Setting CGRU python:
   echo %CGRU_PYTHON%
   set PYTHONHOME=%CGRU_PYTHON%
   set "PATH=%CGRU_PYTHON%;%PATH%"
)
