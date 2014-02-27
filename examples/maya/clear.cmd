echo Clearing Maya examples in %CD%

For /F "Tokens=*" %%I in ('dir /b "scene.mb*"') Do if not "%%I" == "scene.mb" del "%%I"

if exist Keyboard rd /s /q Keyboard
if exist render rd /s /q render
