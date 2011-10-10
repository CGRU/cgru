rem Name=Start Watch...
if defined AF_WATCH_CMD (
   "%AF_WATCH_CMD%"
) else (
   start "Watch Afanasy" "%AF_ROOT%\bin\afwatch.exe"
)
