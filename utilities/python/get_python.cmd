set ver=%1
set arch=%2
if "%ver%"=="" set ver=3.2.3
if "%arch%"=="" set arch=amd64

set location=http://www.python.org/ftp/python
set filename=python-%ver%
if not "%arch%"=="" set filename=%filename%.%arch%
set archive_ext=msi
set archive=%filename%.%archive_ext%
set link=%location%/%ver%/%archive%

if not exist %archive% (
   wget %link%
) else (
   echo Archive %archive% already exists.
)
