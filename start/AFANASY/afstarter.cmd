rem Name=Submit Job...
rem Icon=afanasy.png
rem Separator
call %0\..\_setup.cmd

"%CGRU_PYTHONEXE%" "%CGRU_LOCATION%\utilities\afstarter\afstarter.py" %*
