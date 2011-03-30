#!/bin/bash

ver=4.8.3
location="http://www.riverbankcomputing.co.uk/static/Downloads/PyQt4"
foldername="PyQt-x11-gpl-$ver"
archive_ext="tar.gz"
archive="$foldername.$archive_ext"
link="$location/$archive"

[ -f $archive ] || wget $link

[ -f $archive ] && tar -xvzf $archive
