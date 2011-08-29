#!/bin/bash

# Location:
cgruRoot="../.."
curdir=$PWD

# Version and revision:
packsver=`cat $cgruRoot/version.txt`
pushd $cgruRoot/utilities > /dev/null
packsrev=`python ./getrevision.py ..`
popd > /dev/null

# Exporting current subversion:
cgruSVN=cgru_svn${packsrev}
tmpdir=tmp
[ -d $tmpdir ] || mkdir -pv $tmpdir
cgruSVN_dir=$tmpdir/$cgruSVN
[ -d $cgruSVN_dir ] && rm -rf $cgruSVN_dir
echo "Exporting CGRU revision $packsrev"
svn export $cgruRoot $cgruSVN_dir
cd $tmpdir
acrhivename=$curdir/$cgruSVN.7z
[ -f $acrhivename ] && rm -fv $acrhivename
echo "Compressing $acrhivename"
7za a -r -y -t7z $acrhivename $cgruSVN > /dev/null || echo "Failed!"
