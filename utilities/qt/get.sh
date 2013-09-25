#!/bin/bash

ver=4.8.5
[ -z "$1" ] || ver=$1
lcation="http://download.qt-project.org/official_releases/qt/4.8/$ver"
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
