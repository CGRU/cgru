echo Clearing Blender examples in %CD%

For /F "Tokens=*" %%I in ('dir /b scene.blend*') Do if not "%%I" == "scene.blend" del "%%I"

cd render
del /f /q *
