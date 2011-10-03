@echo off
set folder=%1

For /F "Tokens=*" %%I in ('python getrevision.py %folder%') Do Set CGRU_REVISION=%%I

echo CGRU_REVISION=%CGRU_REVISION%
