set filename=rubyinstaller-1.8.7-p334.exe

set location=http://rubyforge.org/frs/download.php/74293

if exist %filename% (
   echo File %filename% already exists. Skipping download.
) else (
   wget %location%/%filename%
)
