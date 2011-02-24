set filename=rubyinstaller-1.9.1-p430.exe

set location=http://rubyforge.org/frs/download.php/72075

if exist %filename% (
   echo File %filename% already exists. Skipping download.
) else (
   wget %location%/%filename%
)
