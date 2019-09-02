cd "%~dp0"

call "setup.cmd"

set "afservice=%AF_ROOT%\service\afservice.exe render"

if defined AF_SRV_USER if defined AF_SRV_PASS (
	echo AF_SRV_USER=%AF_SRV_USER%
	echo AF_SRV_PASS=%AF_SRV_PASS%
	sc create afservice_render binpath= "%afservice%" type= own start= auto obj= %AF_SRV_USER% password= %AF_SRV_PASS%
) else (
	sc create afservice_render binpath= "%afservice%" type= own start= auto
)

pause