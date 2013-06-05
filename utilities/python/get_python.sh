#!/bin/bash

ver=$1
[ -z "$ver" ] && ver=3.3.2
location="http://www.python.org/ftp/python"
foldername="Python-$ver"
archive_ext="tar.bz2"
archive="$foldername.$archive_ext"
link="$location/$ver/$archive"

[ -f $archive ] || wget $link

[ -f $archive ] && tar -xvjf $archive
