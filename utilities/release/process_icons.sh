#!/bin/bash

# CGRU location:
cgruRoot="../.."

function usage(){
   if [ -n "$ErrorMessage" ]; then
      echo "ERROR: $ErrorMessage"
   fi
   echo "Usage:"
   echo "   `basename $0` AFANASY_BRANCH [clear|rebuild|export_path]"
   echo "Example:"
   echo "   `basename $0` afanasy/trunk rebuild"
   exit
}

# Check Afanasy location:
afanasy=$1
if [ -z $afanasy ]; then
   ErrorMessage="Afanasy branch not specitied."
   usage
fi
if [ ! -d "${cgruRoot}/${afanasy}" ]; then
   ErrorMessage="Afanasy directory '$cgruRoot/$afanasy' does not exists."
   usage
fi

# Get action:
action=$2
if [ -z "${action}" ]; then
   echo 'Building icons...'
elif [ "${action}" == "clear" ]; then
   echo 'Clearing icons...'
elif [ "${action}" == "rebuild" ]; then
   echo 'Rebuilding icons...'
else
   echo "Processing icons for export in '${action}'"
fi

# Icons directories:
iconsdirssrc="\
utilities/release/icons \
doc/icons utilities/regexp/icons \
utilities/moviemaker/icons \
${afanasy}/icons \
${afanasy}/doc/icons \
"
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

   # Just build needed:
   [ -z "${action}" ] && continue

   # Rebuild action:
   [ "${action}" == "rebuild" ] && continue

   # Check export directory:
   dest="${action}/${iconsdir}"
   if [ ! -d "${dest}" ]; then
      ErrorMessage="Icons destination folder '$dest' does not exists."
      usage
   fi

   # Export icons:
   echo "Copying '${iconsdir}' icons to '${dest}'"
   cp -r "${src}/icons" "${dest}"

done
