#!/bin/bash

old=$1
new=$2
#src_old=$3
src_new=$3

function usageExit()
{
	echo "Usage: $0 OLD NEW SOURCE"
	echo "Example: $0 2013 2014 /usr/autodesk/maya2014/scripts"
#	echo "Example: $0 2013 2014 /usr/autodesk/maya2013/scripts /usr/autodesk/maya2014/scripts"
	exit 1
}

[ -z "$src_new" ] && usageExit

if [ ! -d "$old" ]; then
	echo "Old folder '$old' not founded"
	usageExit
fi

#if [ ! -d "$src_old" ]; then
#	echo "Old sources folder '$src_old' not founded"
#	usageExit
#fi

if [ ! -d "$src_new" ]; then
	echo "New sources folder '$src_new' not founded"
	usageExit
fi

[ -d "$new" ] && rm -rfv $new
mkdir -pv $new
#mkdir -pv $new/$old
#mkdir -pv $new/$new

scripts=`ls $old`

for script in $scripts; do
	find "$src_new" -name $script -exec cp -v {} $new \;
done

exit 0

# TRY: make it via diff and patch:
for script in $scripts; do
	find "$src_old" -name $script -exec cp -v {} $new/$old \;
done

for script in $scripts; do
	find "$src_new" -name $script -exec cp -v {} $new/$new \;
done

for script in $scripts; do
	diff $old/$script $new/$old/$script
done

