@echo off

call %CGRU_LOCATION%\software_setup\setup_maya.cmd

if "%GPU_LIST%" == "" (
    "%APP_DIR%\bin\Render.exe" %*
) else (
    "%APP_DIR%\bin\Render.exe" -r redshift -gpu %GPU_LIST% %*
)