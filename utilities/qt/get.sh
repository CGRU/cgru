#!/bin/bash

ver=4.7.2
lcation="http://get.qt.nokia.com/qt/source"
foldername="qt-everywhere-opensource-src-$ver"
archive_ext="tar.gz"
archive="$foldername.$archive_ext"
link="$lcation/$archive"

if [ -f $archive ]; then
	echo "Archive $archive already exists."
else
	wget $link
fi

if [ -d $foldername ]; then
	echo "Folder $foldername already exists."
else
	[ -f $archive ] && tar -xvzf $archive
fi
