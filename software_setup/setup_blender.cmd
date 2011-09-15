set BLENDER_LOCATION=C:\Program Files\Blender Foundation\Blender

rem Setup special Python module:
set PYTHONPATH=%AF_ROOT%\bin_pyaf\3.2;%PYTHONPATH%

set APP_DIR=%BLENDER_LOCATION%
set APP_EXE=%BLENDER_LOCATION%\blender

if exist override.cmd call override.cmd
