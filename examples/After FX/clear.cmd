echo Clearing AfterFX examples in %CD%

For /F "Tokens=*" %%I in ('dir /b "AE Project.aep*"') Do if not "%%I" == "AE Project.aep" del "%%I"

if exist "AE Project.aep Logs" rd /s /q "AE Project.aep Logs"

if exist render rd /s /q render
