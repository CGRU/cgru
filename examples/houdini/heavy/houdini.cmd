PUSHD ..
call setup_houdini.cmd
POPD

"%HOUDINI_LOCATION%\bin\houdini.exe" %*
