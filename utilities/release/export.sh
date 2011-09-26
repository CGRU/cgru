#!/bin/bash

dest=$1
afanasy=trunk

if [ -z "$dest" ]; then
   echo "Specify destination."
   exit 1
fi

function createDir(){
   [ -d $1 ] || mkdir -p $1
}

function copy(){
   createDir $2
   cp $1/* $2/ 2>&1 | grep -v omitting
}

function rcopy(){ rsync -rL --exclude '.svn' --exclude 'override.sh' --exclude 'override.cmd' --exclude '*.pyc' --exclude 'doxygen/output' $1 $2; }

createDir $dest
pushd $dest > /dev/null
dest=$PWD
cd ..
destUP=$PWD
popd > /dev/null

cd ../..

copy . $dest

rcopy bin $dest
rcopy lib $dest
rcopy doc $dest
rcopy examples $dest

copy afanasy/$afanasy $dest/afanasy
copy afanasy/$afanasy/bin $dest/afanasy/bin
[ -d afanasy/$afanasy/bin_pyaf ] && rcopy afanasy/$afanasy/bin_pyaf $dest/afanasy
rcopy afanasy/$afanasy/doc $dest/afanasy
rcopy afanasy/$afanasy/icons $dest/afanasy
rcopy afanasy/$afanasy/init $dest/afanasy
rcopy afanasy/$afanasy/launch $dest/afanasy
rcopy afanasy/$afanasy/plugins $dest/afanasy
rcopy afanasy/$afanasy/python $dest/afanasy
rcopy afanasy/$afanasy/webvisor $dest/afanasy

copy utilities $dest/utilities
rcopy utilities/doc $dest/utilities

createDir $dest/utilities/release
rcopy utilities/release/doc $dest/utilities/release
rcopy utilities/release/icons $dest/utilities/release

createDir $dest/utilities/site
rcopy utilities/site/doc $dest/utilities/site

rcopy utilities/afstarter $dest/utilities
rcopy utilities/keeper $dest/utilities
rcopy utilities/moviemaker $dest/utilities

createDir $dest/utilities/regexp
rcopy utilities/regexp/doc $dest/utilities/regexp
rcopy utilities/regexp/icons $dest/utilities/regexp
rcopy utilities/regexp/bin $dest/utilities/regexp

createDir $dest/plugins
rcopy plugins/nuke $dest/plugins
rcopy plugins/houdini $dest/plugins
rcopy plugins/xsi $dest/plugins
rcopy plugins/blender $dest/plugins
rcopy plugins/max $dest/plugins

copy plugins/maya $dest/plugins/maya
rcopy plugins/maya/doc $dest/plugins/maya
rcopy plugins/maya/icons $dest/plugins/maya
rcopy plugins/maya/mel $dest/plugins/maya
rcopy plugins/maya/mll $dest/plugins/maya

CGRU_VERSION=`cat version.txt`
cd utilities
source ./getrevision.sh ..
echo "${CGRU_VERSION} rev${CGRU_REVISION}" > $dest/version.txt
