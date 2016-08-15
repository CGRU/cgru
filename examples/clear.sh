#!/bin/bash

# This script clears CGRU Examples, delete rendered images and temporary scenes:

examples=$(dirname "$0")
cd "$examples"

echo "Clearing examples in '${examples}'"

script="clear.sh"
for folder in ./*/; do
  pushd $folder > /dev/null
  if [ -x "$script" ]; then
    ./$script
  fi
  popd
done
