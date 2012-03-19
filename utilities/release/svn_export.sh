#!/bin/bash

# Locations:
curdir=$PWD
pushd "../.." > /dev/null
cgruRoot=$PWD
popd > /dev/null

# Version and revision:
packsver=`cat $cgruRoot/version.txt`
pushd $cgruRoot/utilities > /dev/null
packsrev=`python ./getrevision.py $cgruRoot`
popd > /dev/null

# Exporting current subversion:
cgruREV=cgru_rev${packsrev}
tmpdir=tmp
[ -d $tmpdir ] || mkdir -pv $tmpdir
cgruREV_dir=$tmpdir/$cgruREV
[ -d $cgruREV_dir ] && rm -rf $cgruREV_dir
echo "Exporting CGRU revision $packsrev"
svn export $cgruRoot $cgruREV_dir
echo ${packsrev} > $cgruREV_dir/revision.txt
cd $tmpdir
acrhivename=$curdir/cgru.$packsver.7z
[ -f $acrhivename ] && rm -fv $acrhivename
echo "Compressing $acrhivename"
7za a -r -y -t7z $acrhivename $cgruREV > /dev/null || echo "Failed!"
