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

function  copy(){ rsync -qL --exclude 'override.sh' --exclude '*.cmd' --exclude '*.pyc' $1/* $2/; }
function rcopy(){ rsync -rL --exclude '.svn' --exclude 'override.sh' --exclude '*.cmd' --exclude '__pycache__' --exclude '*.pyc' --exclude 'doxygen/output' $1 $2; }

createDir $dest
pushd $dest > /dev/null
dest=$PWD
cd ..
destUP=$PWD
popd > /dev/null

cd ../..
examples/clear.sh

copy . $dest

rcopy bin $dest
rcopy lib $dest
rcopy doc $dest
rcopy icons $dest
rcopy software_setup $dest
rcopy start $dest
rcopy examples $dest

copy afanasy/$afanasy $dest/afanasy
rcopy afanasy/$afanasy/bin $dest/afanasy
[ -d afanasy/$afanasy/bin_pyaf ] && rcopy afanasy/$afanasy/bin_pyaf $dest/afanasy
rcopy afanasy/$afanasy/doc $dest/afanasy
rcopy afanasy/$afanasy/icons $dest/afanasy
rcopy afanasy/$afanasy/init $dest/afanasy
rcopy afanasy/$afanasy/plugins $dest/afanasy
rcopy afanasy/$afanasy/python $dest/afanasy
rcopy afanasy/$afanasy/webvisor $dest/afanasy

copy utilities $dest/utilities
rcopy utilities/doc $dest/utilities

createDir $dest/utilities/release
rcopy utilities/release/doc $dest/utilities/release

createDir $dest/utilities/site
rcopy utilities/site/doc $dest/utilities/site

rcopy utilities/afstarter $dest/utilities
rcopy utilities/keeper $dest/utilities
rcopy utilities/moviemaker $dest/utilities
rcopy utilities/regexp $dest/utilities

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
[ -d plugins/maya/mll ] && rcopy plugins/maya/mll $dest/plugins/maya

CGRU_VERSION=`cat version.txt`
cd utilities
source ./getrevision.sh ..
echo "${CGRU_VERSION}_rev${CGRU_REVISION}" > $dest/version.txt
