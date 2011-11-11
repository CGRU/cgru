echo Clearing Maya examples in %CD%

For /F "Tokens=*" %%I in ('dir /b "scene.mb*"') Do if not "%%I" == "scene.mb" del "%%I"

cd render
del /f /q *
For /F "Tokens=*" %%I in ('dir /b /ad') Do if not "%%I" == ".svn" rd /s /q "%%I"
