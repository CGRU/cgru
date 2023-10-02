rem Source general for all soft directives:
call %CGRU_LOCATION%\software_setup\setup__all.cmd

IF NOT DEFINED LIGHTWAVE_LOCATION (
    For /F "Tokens=*" %%I in ('dir /b "C:\Program Files\NewTek\LightWave_*"') Do set "LIGHTWAVE_LOCATION=C:\Program Files\Newtek\%%I"
) ELSE (
    echo "LIGHTWAVE_LOCATION already set: %LIGHTWAVE_LOCATION%"
)

rem Define location:
set "locate_file=%CGRU_LOCATION%\software_setup\locate_lightwave.cmd"
if exist %locate_file% call %locate_file%

set APP_DIR=%LIGHTWAVE_LOCATION%
set APP_EXE=%LIGHTWAVE_LOCATION%\bin\layout.exe
