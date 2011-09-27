rem Set Maya version:
set MAYA_VERSION=2012
rem Set Maya architecture:
set MAYA_ARCH=-x64
rem For 32bit Maya uncomment next line (or put it in "override.cmd" file)
rem set MAYA_ARCH=

rem Set Maya location:
set MAYA_LOCATION=C:\Program Files\Autodesk\Maya%MAYA_VERSION%

rem Set CGRU Maya scripts location:
set MAYA_CGRU_LOCATION=%CGRU_LOCATION%\plugins\maya

set MAYA_CGRU_MENUS_NAME=CGRU

rem Set custom values if needed here:
IF EXIST override.cmd CALL override.cmd

rem path to modeling, animation, rendering ... menu items
rem if not sel defaut MAYA_CGRU_LOCATION/mel will be used
rem set MAYA_CGRU_MENUS_LOCATION=%MAYA_CGRU_LOCATION%\settings
set MAYA_SCRIPT_PATH=%MAYA_CGRU_LOCATION%\mel\AETemplates;%MAYA_CGRU_LOCATION%\mel\_native\%MAYA_VERSION%;%MAYA_SCRIPT_PATH%
set MAYA_PLUG_IN_PATH=%MAYA_CGRU_LOCATION%\mll\%MAYA_VERSION%%MAYA_ARCH%;%MAYA_PLUG_IN_PATH%
set XBMLANGPATH=%MAYA_CGRU_LOCATION%\icons

rem automatically load plugins located in MAYA_CGRU_LOCATION/mll/MAYA_VERSION directory
set MAYA_CGRU_PLUG_INS_AUTOLOAD=1

rem Add Afanasy scripts:
set MAYA_SCRIPT_PATH=%AF_ROOT%\plugins\maya;%MAYA_SCRIPT_PATH%

rem Add Maya to PATH:
set "PATH=%MAYA_LOCATION%\bin;%PATH%"


set APP_DIR=%MAYA_LOCATION%
set APP_EXE=%MAYA_LOCATION%\bin\maya.exe
