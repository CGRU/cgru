#!/bin/bash

# Get Afanasy location:
curdir=$PWD
cd `dirname $0`
cd ..
afroot=$PWD
cd $curdir

echo "Afanasy location = '$afroot'"

app=$1
remove=$2
daemon="${afroot}/init/afdaemon.sh"
initd="/etc/init.d"

function usage(){
   if [ -n "$ErrorMessage" ]; then
      echo "ERROR: $ErrorMessage"
   fi
   echo "Usage:"
   echo "`basename $0` APPLICATION [rm]"
   exit 0
}

if [ -z $app ]; then
   ErrorMessage="Application is not set."
   usage
fi

if [ "$remove" == "rm" ]; then
   removing="Removing "
else
   appfile="${afroot}/bin/${app}"
   if [ ! -f $appfile ]; then
      ErrorMessage="Application '$appfile' does not exists."
      usage
   fi
   if [ ! -f $daemon ]; then
      ErrorMessage="Daemon script '$daemon' does not exists."
      usage
   fi
fi

echo "${removing}Application = '$app'"

echo "Removing old links:"
update-rc.d -f $app remove
if [ -L $initd/$app ]; then
   rm -fv $initd/$app
fi

if [ "$remove" == "rm" ]; then
   exit
fi

echo "Creating new links:"
ln -sv $daemon $initd/$app
update-rc.d $app defaults 80 20

echo "Done"; exit 0
