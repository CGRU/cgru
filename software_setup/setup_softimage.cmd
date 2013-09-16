rem Source general for all soft directives:
call %CGRU_LOCATION%\software_setup\setup__all.cmd

set XSI_CGRU_PATH=%CGRU_LOCATION%\plugins\xsi

For /F "Tokens=*" %%I in ('dir /b "C:\Program Files\Autodesk\Softimage*"') Do set "XSI_LOCATION=C:\Program Files\Autodesk\%%I\Application"

if defined XSI_PLUGINS (
   set XSI_PLUGINS=%XSI_CGRU_PATH%;%XSI_PLUGINS%
) else (
   set XSI_PLUGINS=%XSI_CGRU_PATH%
)

if defined PYTHONPATH (
   set PYTHONPATH=%XSI_CGRU_PATH%;%PYTHONPATH%
) else (
   set PYTHONPATH=%XSI_CGRU_PATH%
)

set APP_DIR=%XSI_LOCATION%
set APP_EXE=%XSI_LOCATION%\bin\XSI.bat

rem Re-Locate:
set locate_file=%CGRU_LOCATION%\software_setup\locate_softimage.cmd
if exist %locate_file% call %locate_file%

set XSI_LOCATION=%APP_DIR%
