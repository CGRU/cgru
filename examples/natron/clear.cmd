echo Clearing Natron examples in %CD%

For /F "Tokens=*" %%I in ('dir /b "scene.ntp*"') Do if not "%%I" == "scene.ntp" del "%%I"

if exist "*.mov" del /f /q "*.mov"

if exist render rd /s /q render
