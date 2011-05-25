set pyver=%1
if not defined %pyver% set pyver=3.2

rem Check CGRU & Afanasy:
if not exist %CGRU_LOCATION% (
   echo ERROR: CGRU_LOCATION is not defined.
   exit 1
)
if not exist %AF_ROOT% (
   echo ERROR: AF_ROOT is not defined.
   exit 1
)

rem Init Python:
set AF_PYTHON=%CGRU_LOCATION%\utilities\python\%pyver%
if not exist %AF_PYTHON% (
   echo ERROR: %AF_PYTHON% does not exist.
   exit 1
)
set "PATH=%AF_PYTHON%\bin;%PATH%"

rem Python modules path:
set AF_PYTHON=%AF_ROOT%\bin_pyaf\%pyver%;%AF_ROOT%\python3
set PYTHONPATH=%AF_PYTHON%;%PYTHONPATH%
echo PYTHONPATH=%PYTHONPATH%
echo Python3 setup sourced.
