echo Clearing Nuke examples in %CD%

For /F "Tokens=*" %%I in ('dir /b "scene.nk*"') Do if not "%%I" == "scene.nk" del "%%I"

if exist "*.mov" del /f /q "*.mov"

if exist render rd /s /q render
