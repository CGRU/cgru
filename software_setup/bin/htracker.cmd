@echo off

call "%CGRU_LOCATION%\software_setup\setup_houdini.cmd"

set "PYTHONPATH=%HOUDINI_LOCATION%\houdini\python3.9libs;%PYTHONPATH%"

"%HOUDINI_LOCATION%\bin\hython" "%HOUDINI_CGRU_PATH%\htracker.py" %*

