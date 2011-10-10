if [ -z "$CGRU_LOCATION" ]; then
   pushd `dirname "$0"` > /dev/null
   cd ..
   source "./setup.sh"
   popd > /dev/null
fi
