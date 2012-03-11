#!/bin/bash

archive="wget-latest.tar.gz"

if [ -f $archive ]; then
   echo $archive exists. Skipping download.
else
   curl -C - -O http://ftp.gnu.org/gnu/wget/$archive
fi

tar xzfv wget-latest.tar.gz

