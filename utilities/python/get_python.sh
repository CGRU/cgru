#!/bin/bash

ver=$1
location="http://www.python.org/ftp/python"
foldername="Python-$ver"
archive_ext="tar.bz2"
archive="$foldername.$archive_ext"
link="$location/$ver/$archive"

function usageErrorExit(){
   echo "Usage:"
   echo "$0 [python version]"
   exit 1
}

if [ -z "$ver" ] ; then
   echo "Python version is not set."
   usageErrorExit
fi

wget $link

[ -f $archive ] && tar -xvjf $archive
