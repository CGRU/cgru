#!/bin/bash

# Get Afanasy location:
curdir=$PWD
cd `dirname $0`
cd ..
afroot=$PWD
cd $curdir

echo "Afanasy location = '$afroot'"

manager=$1
action=$2
app=$3
daemon="${afroot}/init/afdaemon.sh"
initd="/etc/init.d"

function usage(){
   if [ -n "$ErrorMessage" ]; then
      echo "ERROR: $ErrorMessage"
   fi
   echo "Usage:"
   echo "`basename $0` [u|c|i] [rm|add] APPLICATION"
   echo "'u'   - use 'update-rc.d' (Debian, Ubuntu)"
   echo "'c'   - use 'chkconfig' (CentOS, Fedora, openSUSE)"
   echo "'i'   - use 'insserv' (openSUSE)"
   echo "'add' - add application"
   echo "'rm'  - remove application"
   echo "Examples:"
   echo "`basename $0` c add afrender"
   echo "`basename $0` u rm afserver"
   exit 0
}

if [ -z "${app}" ]; then
   ErrorMessage="Application is not set."
   usage
fi

if [ -z "${manager}" ]; then
   ErrorMessage="Config manager is not set."
   usage
elif [ "${manager}" == "u" ]; then
   echo "Using 'update-rc.d'"
elif [ "${manager}" == "c" ]; then
   echo "Using 'chkconfig'"
elif [ "${manager}" == "i" ]; then
   echo "Using 'insserv'"
else
   ErrorMessage="Unknown config manager '$manager'."
   usage
fi

if [ -z "${action}" ]; then
   ErrorMessage="Action is not set."
   usage
elif [ "$action" == "rm" ]; then
   removing="Removing "
elif [ "$action" == "add" ]; then
   appfile="${afroot}/bin/${app}"
   if [ ! -f $appfile ]; then
      ErrorMessage="Application '$appfile' does not exists."
      usage
   fi
   if [ ! -f $daemon ]; then
      ErrorMessage="Daemon script '$daemon' does not exists."
      usage
   fi
else
   ErrorMessage="Unknown action '$action'."
   usage
fi

echo "${removing}Application = '$app'"
if [ -f $initd/$app ]; then
   echo "Removing old links:"
   [ "${manager}" == "u" ] && update-rc.d -f $app remove
   [ "${manager}" == "c" ] && chkconfig $app off
   [ "${manager}" == "i" ] && insserv -r $app
   rm -fv $initd/$app
fi
if [ "$action" == "rm" ]; then
   exit
fi

# Getting depends:
for depends_file in `ls $afroot/init/depends_${app}_*` ; do
   echo "Processing depends file \"$depends_file\""
   if [ -z "$depends" ]; then
      depends=`cat $depends_file`
   else
      depends="$depends `cat $depends_file`"
   fi
done
echo "Depends = \"$depends\""

# Replacing variables:
echo "Creating new daemon script:"
sed \
-e "s:@APPLICATION@:${app}:g" \
-e "s:@AFROOT@:${afroot}:g" \
-e "s:@DEPENDS@:${depends}:g" \
< "${daemon}" > "${initd}/${app}"

chmod -v a+x ${initd}/${app}

# Creating statrup links:
[ "${manager}" == "u" ] && update-rc.d $app defaults 80 20
[ "${manager}" == "c" ] && chkconfig $app on
[ "${manager}" == "i" ] && insserv $app

echo "Done"; exit 0
