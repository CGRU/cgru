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
tmpdir=/tmp
cgruSVN_dir=$tmpdir/$cgruSVN
echo "Exporting CGRU revision $packsrev"
[ -d $cgruSVN_dir ] && rm -rf $cgruSVN_dir
svn export $cgruRoot $cgruSVN_dir
cd $tmpdir
acrhivename=$curdir/$cgruSVN.7z
[ -f $acrhivename ] && rm -fv $acrhivename
7za a -r -y -t7z $acrhivename $cgruSVN > /dev/null || echo "Failed!"
