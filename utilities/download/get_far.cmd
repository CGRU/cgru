set filename=Far20b1807.x86.20110203.msi

set location=http://www.farmanager.com/files

if exist %filename% (
   echo File %filename% already exists. Skipping download.
) else (
   wget %location%/%filename%
)
