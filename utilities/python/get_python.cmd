set ver=%1
set arch=%2
if "%ver%"=="" set ver=2.5.4

set location=http://www.python.org/ftp/python
set foldername=python-%ver%
if not "%arch%"=="" set foldername=%foldername%.%arch%
set archive_ext=msi
set archive=%foldername%.%archive_ext%
set link=%location%/%ver%/%archive%

if not exist %archive% (
   wget %link%
) else (
   echo Archive %archive% already exists.
)
