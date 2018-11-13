call setup.cmd

set "afservice=%AF_ROOT%\service\afservice.exe server"

sc create afserver binpath= "%afservice%" type= own start= auto

pause