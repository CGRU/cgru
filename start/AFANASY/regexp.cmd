rem Name=Check RegExp...
rem Icon=regexp.png

call %0\..\_setup.cmd

start "Check RegExp" "%CGRU_PYTHONDIR%\pythonw.exe" "%CGRU_LOCATION%\utilities\regexp\regexp.py" %*
