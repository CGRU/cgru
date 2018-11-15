call setup.cmd

set "afservice=%AF_ROOT%\service\afservice.exe server"

sc create afservice_server binpath= "%afservice%" type= own start= auto

pause