set ver=1.6.15-x64

set filename=Slik-Subversion-%ver%.msi

set location=http://www.sliksvn.com/pub

if exist %filename% (
   echo File %filename% already exists. Skipping download.
) else (
   wget %location%/%filename%
)
