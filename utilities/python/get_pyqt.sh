#!/bin/bash

ver=4.10.1
location="http://sourceforge.net/projects/pyqt/files/PyQt4/PyQt-$ver"
foldername="PyQt-x11-gpl-$ver"
archive_ext="tar.gz"
archive="$foldername.$archive_ext"
link="$location/$archive"

[ -f $archive ] || wget $link

[ -f $archive ] && tar -xvzf $archive
