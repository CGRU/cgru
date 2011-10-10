rem Name=Start Render...
if defined AF_RENDER_CMD (
   "%AF_RENDER_CMD%"
) else (
   "%AF_ROOT%\bin\afrender.exe"
)
