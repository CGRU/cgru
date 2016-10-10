#!/bin/bash

cd `dirname "$0"`

cd ..
cd ..

source "./setup.sh" > /dev/null

"${CGRU_LOCATION}/rules/bin/thumbnail.py" "$@"

