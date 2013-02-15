rem Source general for all soft directives:
call %CGRU_LOCATION%\software_setup\setup__all.cmd

For /F "Tokens=*" %%I in ('dir /b "C:\Program Files\Side Effects Software\Houdini*"') Do set "HOUDINI_LOCATION=C:\Program Files\Side Effects Software\%%I"

set APP_DIR=%HOUDINI_LOCATION%
set APP_EXE=%HOUDINI_LOCATION%\bin\houdini.exe

rem Define location:
set locate_file=%CGRU_LOCATION%\software_setup\locate_houdini.cmd
if exist %locate_file% call %locate_file%

set HOUDINI_LOCATION=%APP_DIR%

set HOUDINI_CGRU_PATH=%CGRU_LOCATION%\plugins\houdini

set HOUDINI_AF_PATH=%AF_ROOT%\plugins\houdini

set HOUDINI_AF_OTLSCAN_PATH=%HOUDINI_AF_PATH%;%HOUDINI_LOCATION%\houdini\otls

set PYTHONPATH=%HOUDINI_AF_PATH%;%PYTHONPATH%

IF DEFINED HOUDINI_OTLSCAN_PATH (
   set HOUDINI_OTLSCAN_PATH=%HOUDINI_AF_OTLSCAN_PATH%;%HOUDINI_OTLSCAN_PATH%
) ELSE (
   set HOUDINI_OTLSCAN_PATH=%HOUDINI_AF_OTLSCAN_PATH%
)
