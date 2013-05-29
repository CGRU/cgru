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
mput *.html *.php *.txt
ls

mkdir lib
mput lib/*.css lib/*.js
ls lib

mkdir rules
mput rules/*.css rules/*.js
mput rules/rules.00_general.json
ls rules

mkdir rules/bin
mput rules/bin/*.py
ls rules/bin

mkdir rules/assets
mput rules/assets/*.js rules/assets/*.html
ls rules/assets



mkdir rules_root
cd rules_root
lcd rules_root

mkdir .rules
mput .rules/*.html .rules/*.json
ls .rules

mkdir Ask_Questions_Here
mkdir Ask_Questions_Here/.rules
mput Ask_Questions_Here/.rules/*.html Ask_Questions_Here/.rules/*.json
ls Ask_Questions_Here/.rules


END_SCRIPT
