set ver=4.8.4

set location=http://releases.qt-project.org/qt4/source
set foldername=qt-everywhere-opensource-src-%ver%
set archive_ext=zip
set archive=%foldername%.%archive_ext%
set link=%location%/%archive%

if exist %foldername% (
   echo %archive% already exists, exiting...
   exit
)

if not exist %archive% (
   wget %link%
) else (
   echo Archive %archive% already exists, skipping download...
)

7z x %archive%
