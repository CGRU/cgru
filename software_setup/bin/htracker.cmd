@echo off

call "%CGRU_LOCATION%\software_setup\setup_houdini.cmd"

set "PYTHONPATH=%HOUDINI_LOCATION%\houdini\python2.7libs;%PYTHONPATH%"

"%HOUDINI_LOCATION%\bin\hython2.7" "%HOUDINI_CGRU_PATH%\htracker.py" %*

