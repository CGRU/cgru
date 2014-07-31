#!/bin/bash

cd `dirname $0`
cd ..

dest=$1
[ -z "$dest" ] && dest=cgru.info
cgru=$PWD

credentials=rules/export_credentials.sh
if [ ! -f $credentials ]; then
	echo $credentials file not found
	exit 1
fi
source $credentials

echo USER=$FTP_USER
echo DEST=$dest

ftp -pin $dest <<END_SCRIPT
quote USER $FTP_USER
quote PASS $FTP_PASS

cd www
cd cgru.info

mkdir rules
cd rules
mput *.html *.php *.txt
ls

mkdir lib
mput lib/*.css
ls lib

mkdir lib/js
mput lib/js/*.js
ls lib/js

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
mput .rules/*.html .rules/rules.*.json
ls .rules

mkdir Ask_Questions_Here
mkdir Ask_Questions_Here/.rules
mput Ask_Questions_Here/.rules/body.html Ask_Questions_Here/.rules/rules.*.json
ls Ask_Questions_Here/.rules

mkdir Ask_Questions_Here/AFANASY
mkdir Ask_Questions_Here/AFANASY/.rules
mput Ask_Questions_Here/AFANASY/.rules/body.html Ask_Questions_Here/AFANASY/.rules/rules.*.json
ls Ask_Questions_Here/AFANASY/.rules

mkdir CG_PROJECT
cd CG_PROJECT
lcd CG_PROJECT
mkdir .rules
mput .rules/body.html
ls .rules

mkdir SCENES
cd SCENES
lcd SCENES
mkdir .rules
mput .rules/body.html
ls .rules

mkdir A_SCENE
cd A_SCENE
lcd A_SCENE
mkdir .rules
mput .rules/body.html .rules/rules.*.json

ls .rules

mkdir A_SHOT_01
mkdir A_SHOT_01/.rules
mput A_SHOT_01/.rules/body.html
ls A_SHOT_01/.rules

mkdir A_SHOT_02
mkdir A_SHOT_02/.rules
mput A_SHOT_02/.rules/body.html
ls A_SHOT_02/.rules

mkdir _readme_
mkdir _readme_/.rules
mput _readme_/.rules/body.html
ls _readme_/.rules

END_SCRIPT
