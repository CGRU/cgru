set pyver=%1

if not defined %1 set pyver=3.2.1

rem Check CGRU:
if not exist %CGRU_LOCATION% (
   echo ERROR: CGRU_LOCATION does not exist:
   echo %CGRU_LOCATION%
   exit
)

rem Check Afanasy:
if not exist %AF_ROOT% (
   echo ERROR: AF_ROOT does not exist:
   echo %AF_ROOT%
   exit
)

rem Init Python if exists:
set cgru_python=%CGRU_LOCATION%\utilities\python\%pyver%
if exist %cgru_python% set "PATH=%cgru_python%\bin;%PATH%"

rem Python modules path:
set pyafdir=%AF_ROOT%\bin_pyaf\%pyver%
set pyaf=%pyafdir%\pyaf.pyd
if not exist %pyaf% (
   echo ERROR: Python module not founded:
   echo %pyaf%
   exit
)

rem Python scripts path:
set afpython3=%AF_ROOT%\python3
if not exist %afpython3% (
   echo ERROR: Afanasy Python3 scripts does not exist:
   echo %afpython3%
   exit
)

rem Define result variables:
set AF_PYTHON=%pyafdir%;%afpython3%
echo AF_PYTHON=%PYTHONPATH%
set PYTHONPATH=%AF_PYTHON%;%PYTHONPATH%
echo Python3 setup sourced.
