PUSHD ..
call setup_houdini.cmd
POPD

"%HOUDINI_LOCATION%\bin\hython.exe" %HOUDINI_CGRU_PATH%\hrender_af.py %*
