PUSHD ..
call setup.cmd
POPD

set PYTHONPATH=%AF_ROOT%\bin_pyaf\2.6.6;%PYTHONPATH%

set XSI_CGRU_PATH=%CGRU_LOCATION%\plugins\xsi
set XSI_AF_PATH=%AF_ROOT%\plugins\xsi

set XSI_LOCATION=C:\Program Files\Autodesk\Softimage 2012

if exist override.cmd call override.cmd

if defined XSI_PLUGINS (
   set XSI_PLUGINS=%XSI_CGRU_PATH%;%XSI_PLUGINS%
) else (
   set XSI_PLUGINS=%XSI_CGRU_PATH%
)

if defined PYTHONPATH (
   set PYTHONPATH=%XSI_AF_PATH%;%PYTHONPATH%
) else (
   set PYTHONPATH=%XSI_AF_PATH%
)
