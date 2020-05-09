rem Name=Submit Job...
rem Icon=afanasy.png

call %0\..\_setup.cmd

"%CGRU_PYTHONEXE%" "%CGRU_LOCATION%\utilities\afstarter\afstarter.py" %*
