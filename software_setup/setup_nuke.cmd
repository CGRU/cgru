rem Source general for all soft directives:
call %CGRU_LOCATION%\software_setup\setup__all.cmd

rem Setup CGRU common scripts if location provided
set NUKE_CGRU_PATH=%CGRU_LOCATION%\plugins\nuke
if defined CGRU_LOCATION (
   if defined NUKE_PATH (
      set NUKE_PATH=%NUKE_CGRU_PATH%;%NUKE_PATH%
   ) else (
      set NUKE_PATH=%NUKE_CGRU_PATH%
   )
)

rem Default number of threads for rendering:
rem SET NUKE_AF_RENDERTHREADS=2

rem Nuke render launcher:
SET NUKE_AF_RENDER=nuke


IF NOT DEFINED NUKE_LOCATION (
   For /F "Tokens=*" %%I in ('dir /b "C:\Program Files\Nuke*"') Do set "NUKE_LOCATION=C:\Program Files\%%I"
) ELSE (
   echo "NUKE_LOCATION already set: %NUKE_LOCATION%"
)

For /F "Tokens=*" %%I in ('dir /b "%NUKE_LOCATION%\Nuke*.*.exe"') Do set "NUKE_EXEC=%NUKE_LOCATION%\%%I"
rem echo %NUKE_EXEC%

SET NUKE_DIR=%NUKE_LOCATION%
SET APP_DIR=%NUKE_LOCATION%
SET APP_EXE=%NUKE_EXEC%

rem Locate:
set locate_file=%CGRU_LOCATION%\software_setup\locate_nuke.cmd
if exist %locate_file% call %locate_file%

SET NUKE_LOCATION=%APP_DIR%

SET LM_LICENSE_FILE=%NUKE_LOCATION%\nuke.lic
SET LM_LICENSE_FILE=%NUKE_LOCATION%\foundry.lic
