set ver=5.9.3

set location=http://download.tuxfamily.org/notepadplus/%ver%
set filename=npp.%ver%.Installer.exe

if exist %filename% (
   echo File %filename% already exists. Skipping download.
) else (
   wget %location%/%filename%
)
