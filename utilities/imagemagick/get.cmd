set filename=ImageMagick-windows.zip

set location=ftp://ftp.imagemagick.org/pub/ImageMagick/windows

if exist %filename% (
   echo File %filename% already exists. Skipping download.
) else (
   wget %location%/%filename%
)

7z x %filename%

rem #define MAGICKCORE_OPENEXR_DELEGATE
rem #define MAGICKCORE_HDRI_SUPPORT
rem zlibwapi.lib;Imath.lib;IlmThread.lib;IlmImf.lib;Iex.lib;Half.lib;
rem rem #define MAGICKCORE_QUANTUM_DEPTH 32
