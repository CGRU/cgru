call setup_cgru.cmd
call setup_afanasy.cmd

set HOUDINI_LOCATION=C:\Program Files\Side Effects Software\Houdini 10.0.554

if exist override.cmd call override.cmd

set HOUDINI_AF_VERVION=10
set HOUDINI_AF_PATH=%AF_ROOT%\plugins\houdini\houdini%HOUDINI_AF_VERVION%

set HOUDINI_AF_OTLSCAN_PATH=%HOUDINI_AF_PATH%\otls;%HOUDINI_LOCATION%\houdini\otls

IF DEFINED HOUDINI_OTLSCAN_PATH (
   set HOUDINI_OTLSCAN_PATH=%HOUDINI_AF_OTLSCAN_PATH%;%HOUDINI_OTLSCAN_PATH%
) ELSE (
   set HOUDINI_OTLSCAN_PATH=%HOUDINI_AF_OTLSCAN_PATH%
)
