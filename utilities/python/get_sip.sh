#!/bin/bash

ver=4.14.6
location="http://sourceforge.net/projects/pyqt/files/sip"
foldername="sip-$ver"
archive_ext="tar.gz"
archive="$foldername.$archive_ext"
link="$location/$foldername/$archive"

[ -f $archive ] || wget $link

[ -f $archive ] && tar -xvzf $archive
