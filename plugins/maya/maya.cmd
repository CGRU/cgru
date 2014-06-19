rem Set Maya version:
set MAYA_VERSION=2015

rem Set Maya location:
set MAYA_LOCATION=C:\Program Files\Autodesk\Maya%MAYA_VERSION%

rem Set CGRU Maya scripts location:
set MAYA_CGRU_LOCATION=%CD%

set MAYA_CGRU_MENUS_NAME=CGRU

IF EXIST override.cmd CALL override.cmd

rem path to modelling, animation, rendering ... menu items
rem if not set default MAYA_CGRU_LOCATION/mel will be used
rem set MAYA_CGRU_MENUS_LOCATION=%MAYA_CGRU_LOCATION%\settings
set MAYA_SCRIPT_PATH=%MAYA_CGRU_LOCATION%\mel\AETemplates;%MAYA_SCRIPT_PATH%
set MAYA_PLUG_IN_PATH=%MAYA_CGRU_LOCATION%\mll\%MAYA_VERSION%;%MAYA_PLUG_IN_PATH%
set XBMLANGPATH=%MAYA_CGRU_LOCATION%\icons

set "PATH=%MAYA_LOCATION%\bin;%PATH%"

rem Enable Afanasy if it is set:
IF DEFINED AF_ROOT set "MAYA_SCRIPT_PATH=%MAYA_CGRU_LOCATION%\afanasy;%MAYA_SCRIPT_PATH%"

start maya.exe %*
