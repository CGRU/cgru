echo Clearing Clarisse examples in %CD%

if exist "scene.af*.render" del /f /q "scene.af*.render"

if exist "*.autosave" del /f /q "*.autosave"

if exist render rd /s /q render
