rem Name=Start Watch...
rem Icon=afwatch.png

call %0\..\_setup.cmd
if defined AF_WATCH_CMD (
   start "Watch Afanasy" "%AF_WATCH_CMD%" %*
) else (
   start "Watch Afanasy" "%AF_ROOT%\bin\afwatch.exe" %*
)
