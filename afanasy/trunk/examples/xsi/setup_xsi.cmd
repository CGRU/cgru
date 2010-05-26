call setup_afanasy.cmd

set XSI_AF_PATH=%AF_ROOT%\plugins\xsi

if defined PYTHONPATH (
   set PYTHONPATH=%XSI_AF_PATH%;%PYTHONPATH%
) else (
   set PYTHONPATH=%XSI_AF_PATH%
)

set XSI_LOCATION=C:\Program Files\Autodesk\Softimage 2011

if exist override.cmd call override.cmd
