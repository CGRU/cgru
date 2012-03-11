set filename=ImageMagick-windows.zip

set location=ftp://ftp.imagemagick.org/pub/ImageMagick/windows

if exist %filename% (
   echo File %filename% already exists. Skipping download.
) else (
   wget %location%/%filename%
)

7z x %filename%

set config=ImageMagick-6.7.1\VisualMagick\magick\magick-config.h.in
if not exist %config% (
   echo Config file "%config%" not founded.
   exit
)

echo off

echo Patching "%config%"
echo #define MAGICKCORE_OPENEXR_DELEGATE >> %config%
echo #define MAGICKCORE_HDRI_SUPPORT >> %config%

rem zlibwapi.lib;Imath.lib;IlmThread.lib;IlmImf.lib;Iex.lib;Half.lib;
rem rem #define MAGICKCORE_QUANTUM_DEPTH 32
echo Done.
