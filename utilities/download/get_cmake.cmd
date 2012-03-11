set filename=cmake-2.8.5-win32-x86.exe

set location=http://www.cmake.org/files/v2.8

if exist %filename% (
   echo File %filename% already exists. Skipping download.
) else (
   wget %location%/%filename%
)
