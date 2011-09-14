set BLENDER_LOCATION="C:\Program Files\Blender Foundation\Blender"

rem Setup special Python module:
set PYTHONPATH=%AF_ROOT%\bin_pyaf\3.2;%PYTHONPATH%

if exist override.cmd call override.cmd
