set packages=ilmbase-1.0.2 openexr-1.7.0
set location=http://download.savannah.nongnu.org/releases/openexr
echo off

for %%P in (%packages%) do ( call :do_get "%%P")
GOTO :eof

:do_get
set package=%1
echo Package=%package%
if exist %package% (
    echo %package% already exists, skipping...
    GOTO :eof
)
set filename=%package%.tar.gz
if exist %filename% (
   echo File %filename% already exists. Skipping download.
) else (
   wget %location%/%filename%
)
7z x %filename%
set filename=%package%.tar
7z x %filename%
del /f /q /s %filename%
