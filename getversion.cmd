@echo off
set folder=%1

For /F "Tokens=*" %%I in ('python getrevision.py %folder%') Do Set CGRU_VERSION=%%I

echo CGRU_VERSION %CGRU_VERSION%
