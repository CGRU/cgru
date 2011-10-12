if [ -z "$CGRU_LOCATION" ]; then
   if [ -d `dirname "$0"` ]; then
      pushd `dirname "$0"` > /dev/null
      pushed="1"
   fi
   cd ..
   source "./setup.sh"
   [ -z "$pushed" ] || popd > /dev/null
fi
