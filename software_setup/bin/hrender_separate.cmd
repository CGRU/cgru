@echo off

call %CGRU_LOCATION%\software_setup\setup_houdini.cmd

python %HOUDINI_CGRU_PATH%\hrender_separate.py %*
