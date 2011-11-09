@echo off

call %CGRU_LOCATION%\software_setup\setup_houdini.cmd

hython %HOUDINI_CGRU_PATH%\hrender_af.py %*
