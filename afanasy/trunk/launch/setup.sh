#!/bin/bash

afanasy=$0
afanasy="`dirname $afanasy`"
afanasy="`dirname $afanasy`"

pushd "$afanasy" >> /dev/null
source ./setup.sh
popd >> /dev/null
