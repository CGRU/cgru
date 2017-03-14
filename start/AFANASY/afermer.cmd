rem Name=Start AFermer...
rem Icon=afermer.png

call %0\..\_setup.cmd
if defined AF_WATCH_CMD (
   start "AFermer" "%AF_FERMER_CMD%" %*
) else (
   start "AFermer" "%AF_ROOT%\bin\afermer.exe" %*
)
