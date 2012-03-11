#!/bin/bash

username=$1
[ -z "$1" ] && username=`whoami`

cd temp/wwwroot

rsync -av --delete * $username,cgru@web.sourceforge.net:htdocs
