rem if not defined CGRU_PYTHONEXE (
echo "CGRU_PYTHON"
   set python=%CGRU_LOCATION%\python
   if exist %python% (
      echo Using CGRU Python: %python%
      set "PATH=%python%;%PATH%"
      set CGRU_PYTHONDIR=%python%
      set CGRU_PYTHONEXE=%python%\python.exe
   ) else (
      set CGRU_PYTHONEXE=python
   )
rem )
