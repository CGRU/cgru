@echo off

call %CGRU_LOCATION%\software_setup\setup_c4d.cmd
echo %*
%CGRU_PYTHONEXE% %C4D_RENDER_SCRIPT% %*