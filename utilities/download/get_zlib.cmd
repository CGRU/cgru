set filename=zlib-1.2.5.tar.gz
set location=http://zlib.net
if exist %filename% (
   echo File %filename% already exists. Skipping download.
) else (
   wget %location%/%filename%
)

set filename=zlib125dll.zip
set location=http://www.winimage.com/zLibDll
if exist %filename% (
   echo File %filename% already exists. Skipping download.
) else (
   wget %location%/%filename%
)
