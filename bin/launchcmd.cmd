set PYTHONHOME=C:\bz\bin\python\25_32
set PATH=%PYTHONHOME%;%PATH%

set ENCODE_BINS=C:\bz\bin\ImageMagick-6.6.3-4
set PATH=%ENCODE_BINS%;%PATH%

cd C:\bz\bin\cgru\utilities\moviemaker

python C:\bz\bin\cgru\bin\launchcmd.py %*
