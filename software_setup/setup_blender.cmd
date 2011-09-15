set BLENDER_LOCATION=C:\Program Files\Blender Foundation\Blender

rem Setup special Python module:
rem set PYTHONPATH=%AF_ROOT%\bin_pyaf\3.2.2;%PYTHONPATH%
set AF_PYTHON=%AF_ROOT%\bin_pyaf\3.2.2;%AF_ROOT%\python

set APP_DIR=%BLENDER_LOCATION%
set APP_EXE=%BLENDER_LOCATION%\blender

if exist override.cmd call override.cmd
