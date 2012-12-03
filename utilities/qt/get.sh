#!/bin/bash

ver=4.8.4
[ -z "$1" ] || ver=$1
lcation="http://releases.qt-project.org/qt4/source"
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
