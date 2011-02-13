#!/bin/bash

ver=4.7.1
lcation="http://get.qt.nokia.com/qt/source"
foldername="qt-everywhere-opensource-src-$ver"
archive_ext="tar.gz"
archive="$foldername.$archive_ext"
link="$lcation/$archive"

wget $link

[ -f $archive ] && tar -xvzf $archive
