call setup.cmd

set "afservice=%AF_ROOT%\service\afservice.exe render"

sc create afrender binpath= "%afservice%" type= own start= auto

pause