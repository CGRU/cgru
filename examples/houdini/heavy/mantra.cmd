PUSHD ..
call setup_houdini.cmd
POPD

"%HOUDINI_LOCATION%\bin\mantra.exe" %*
