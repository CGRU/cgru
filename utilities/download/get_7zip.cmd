set ver=920-x64

set location=http://downloads.sourceforge.net/sevenzip
set filename=7z%ver%.msi

if exist %filename% (
   echo File %filename% already exists. Skipping download.
) else (
   wget %location%/%filename%
)
