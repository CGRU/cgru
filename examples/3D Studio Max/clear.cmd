echo Clearing Max examples in %CD%

For /F "Tokens=*" %%I in ('dir /b scene.max*') Do if not "%%I" == "scene.max" del "%%I"

if exist render rd /s /q render
