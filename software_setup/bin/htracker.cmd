@echo off

call "%CGRU_LOCATION%\software_setup\setup_houdini.cmd"

set "PYTHONPATH=%HFS%\houdini\python2.7libs;%PYTHONPATH%"

"%HFS%\python\bin\python2.7" "%HOUDINI_CGRU_PATH%\htracker.py" %*

