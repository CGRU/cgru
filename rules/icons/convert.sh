#!/bin/bash

img=$1


pushd ../.. > /dev/null
source "./setup.sh"
popd > /dev/null

icon=`basename "$img"`

convert "$img" -colorspace gray -thumbnail 48x48 "$icon"

