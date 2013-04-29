#!/bin/bash

cd `dirname $0`
cd ..

dest=$1
[ -z "$dest" ] && dest=cgru.info
cgru=$PWD

credentials=rules/export_credentials.sh
if [ ! -f $credentials ]; then
	echo $credentials file not founded
	exit 1
fi
source $credentials

echo USER=$FTP_USER
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
