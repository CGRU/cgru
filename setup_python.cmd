set python=%CGRU_LOCATION%\python
if exist %python% (
   echo Using CGRU Python: %python%
   set PYTHONHOME=%python%
   set "PATH=%python%;%PATH%"
   set CGRU_PYTHONDIR=%python%
rem   set CGRU_PYTHONEXE=%python%\python.exe
   set CGRU_PYTHONEXE=python
) else (
   set CGRU_PYTHONEXE=python
)
