rem Name=Check RegExp...
call %0\..\_setup.cmd

start "Check RegExp" "%CGRU_PYTHONDIR%\pythonw.exe" "%CGRU_LOCATION%\utilities\regexp\regexp.py" %*
