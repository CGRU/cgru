#!/bin/bash

ver=$1
[ -z "$ver" ] && ver=2.7.1
location="http://www.python.org/ftp/python"
foldername="Python-$ver"
archive_ext="tar.bz2"
archive="$foldername.$archive_ext"
link="$location/$ver/$archive"

wget $link

[ -f $archive ] && tar -xvjf $archive
