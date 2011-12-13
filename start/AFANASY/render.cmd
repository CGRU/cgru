rem Name=Local Render...
call %0\..\_setup.cmd
if defined AF_RENDER_CMD (
   "%AF_RENDER_CMD%" %*
) else (
   "%AF_ROOT%\bin\afrender.exe" %*
)
