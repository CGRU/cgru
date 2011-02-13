#!/bin/bash

ver=4.8.3
lcation="http://www.riverbankcomputing.co.uk/static/Downloads/PyQt4"
foldername="PyQt-x11-gpl-$ver"
archive_ext="tar.gz"
archive="$foldername.$archive_ext"
link="$lcation/$archive"

wget $link

[ -f $archive ] && tar -xvjf $archive
