#!/bin/bash

cd `dirname $0`
cd ..

dest=$1
[ -z "$dest" ] && dest=cgru.info
cgru=$PWD

#exts="js css html php txt"

source rules/export_credentials.sh

echo DEST=$dest

ftp -in $dest <<END_SCRIPT
quote USER $FTP_USER
quote PASS $FTP_PASS

mkdir rules
cd rules

mkdir lib
cd lib
lcd lib
mput *.css *.js
ls

cdup
lcd ..
mput *.html *.php *.txt
ls

mkdir rules
cd rules
lcd rules
mput *.html *.css *.js *.json
ls
END_SCRIPT
