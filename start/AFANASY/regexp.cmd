rem Name=Check RegExp...
call %0\..\_setup.cmd

"%CGRU_PYTHONEXE%" "%CGRU_LOCATION%\utilities\regexp\regexp.py" %*
