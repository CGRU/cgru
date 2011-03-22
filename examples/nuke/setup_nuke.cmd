PUSHD ..
call setup.cmd
POPD

rem Setup CGRU common scripts if location provided
set NUKE_CGRU_PATH=%CGRU_LOCATION%\plugins\nuke
if defined CGRU_LOCATION (
   if defined NUKE_PATH (
      set NUKE_PATH=%NUKE_CGRU_PATH%;%NUKE_PATH%
   ) else (
      set NUKE_PATH=%NUKE_CGRU_PATH%
   )
)

rem Setup afanasy addons:
SET NUKE_AF_VERSION=python
SET NUKE_AF_PATH=%AF_ROOT%\plugins\nuke\%NUKE_AF_VERSION%
if defined NUKE_PATH (
   set NUKE_PATH=%NUKE_AF_PATH%;%NUKE_PATH%
) else (
   set NUKE_PATH=%NUKE_AF_PATH%
)

rem Add Afanasy Python module:
set PYTHONPATH=%AF_ROOT%\bin_pyaf\2.6.6;%PYTHONPATH%

rem Default number of threads for rendering:
SET NUKE_AF_RENDERTHREADS=2

rem Nuke render launcher:
SET NUKE_AF_RENDER=nuke -i -m AF_THREADS
if defined CGRU_LOCATION (
   SET NUKE_AF_RENDER=%NUKE_AF_RENDER% -t %CGRU_LOCATION%\plugins\nuke\render.py
)

SET NUKE_DIR=C:\Program Files\Nuke6.2v1
SET NUKE_EXEC=%NUKE_DIR%\Nuke6.2.exe
SET LM_LICENSE_FILE=%NUKE_DIR%\nuke.lic
SET LM_LICENSE_FILE=%NUKE_DIR%\foundry.lic

IF EXIST override.cmd CALL override.cmd
