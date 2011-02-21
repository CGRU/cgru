set ver=4.7.1

set location=http://get.qt.nokia.com/qt/source
set foldername=qt-everywhere-opensource-src-%ver%
set archive_ext=zip
set archive=%foldername%.%archive_ext%
set link=%location%/%archive%

if not exist %archive% (
   wget %link%
) else (
   echo Archive %archive% already exists.
)
