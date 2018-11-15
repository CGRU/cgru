call setup.cmd

set "afservice=%AF_ROOT%\service\afservice.exe render"

sc create afservice_render binpath= "%afservice%" type= own start= auto

pause