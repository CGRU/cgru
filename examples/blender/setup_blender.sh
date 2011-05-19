#!/bin/bash

pushd .. >> /dev/null
source ./setup.sh
popd >> /dev/null

[ -f override.sh ] && source override.sh

afscripts_cgru=$AF_ROOT/plugins/blender
afscripts_blender=$HOME/.blender/scripts/afanasy
if [ -L $afscripts_blender ]; then
   if [ "`readlink $afscripts_blender`" != $afscripts_cgru ]; then
      rm -vf $afscripts_blender
   fi
fi
if [ ! -d $afscripts_blender ]; then
   echo "Linking Afanasy scripts folder:"
   ln -svf $afscripts_cgru $afscripts_blender
fi
