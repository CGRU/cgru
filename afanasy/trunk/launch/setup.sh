#!/bin/bash

pushd `dirname $0`

afanasy=$PWD
afanasy="`dirname $afanasy`"
cd "$afanasy"
source ./setup.sh

popd >> /dev/null
