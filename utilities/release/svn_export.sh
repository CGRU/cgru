#!/bin/bash

# Locations:
curdir=$PWD
pushd "../.." > /dev/null
cgruRoot=$PWD
popd > /dev/null

# Version and revision:
version=`cat $cgruRoot/version.txt`
pushd $cgruRoot/utilities > /dev/null
revision=`python ./getrevision.py $cgruRoot`
popd > /dev/null

# Prepare folders:
snapshot=cgru.$version
tmpdir=tmp
[ -d $tmpdir ] || mkdir -pv $tmpdir
snapshot_dir=$tmpdir/$snapshot
[ -d $snapshot_dir ] && rm -rf $snapshot_dir

# Exporting current subversion:
echo "Exporting CGRU revision $revision"
svn export $cgruRoot $snapshot_dir
echo ${revision} > $snapshot_dir/revision.txt

# Make an archive:
cd $tmpdir
archivename=$curdir/$snapshot.zip
[ -f $archivename ] && rm -fv $archivename
echo Compressing "${snapshot}" to "${archivename}"
zip -y -r "${archivename}" "${snapshot}" -x "*/.*"

