#!/bin/bash

# CGRU location:
cgruRoot="../.."

function usage(){
   if [ -n "$ErrorMessage" ]; then
      echo "ERROR: $ErrorMessage"
   fi
   echo "Usage:"
   echo "   `basename $0` [clear|rebuild]"
   echo "Example:"
   echo "   `basename $0` rebuild"
   exit
}

# Get action:
action=$1
if [ -z "${action}" ]; then
   echo 'Building icons...'
elif [ "${action}" == "clear" ]; then
   echo 'Clearing icons...'
elif [ "${action}" == "rebuild" ]; then
   echo 'Rebuilding icons...'
fi

# Icons directories:
iconsdirssrc="icons"
for iconsdir in $iconsdirssrc; do

   # Check icons directory:
   src="${cgruRoot}/${iconsdir}"
   if [ ! -d "$src" ]; then
      ErrorMessage="Icons source folder '$src' does not exists."
      usage
   fi

   # Just clear icons:
   if [ "${action}" == "clear" ]; then
      rm -rvf "${src}/icons"
      continue
   fi

   # Clear icons:
   [ "${action}" == "rebuild" ] && rm -rvf "${src}/icons"

   # Generate png icons from svg if was not:
   if [ ! -d "${src}/icons" ]; then
      echo "Generating icons in '${src}':"
      tmp=$PWD
      cd $src
      ./make.sh   
      cd $tmp
   fi

done
