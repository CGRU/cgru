rem Source general for all soft directives:
call %CGRU_LOCATION%\software_setup\setup__all.cmd

rem Setup CGRU common scripts if location provided
set "FUSION_CGRU_PATH=%CGRU_LOCATION%\plugins\fusion"
if defined CGRU_LOCATION (
   if defined FUSION_PATH (
      set "FUSION_PATH=%NUKE_CGRU_PATH%;%NUKE_PATH%"
   ) else (
      set "FUSION_PATH=%NUKE_CGRU_PATH%"
   )
)

SET "APP_DIR=C:\Program Files\Blackmagic Design\Fusion"
SET "APP_EXE=%APP_DIR%\Fusion.exe"

rem Locate:
set "locate_file=%CGRU_LOCATION%\software_setup\locate_fusion.cmd"
if exist %locate_file% call %locate_file%
