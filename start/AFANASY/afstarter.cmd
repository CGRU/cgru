rem Name=Submit Job...
call %0\..\_setup.cmd

"%CGRU_PYTHONEXE%" "%CGRU_LOCATION%\utilities\afstarter\afstarter.py" %*
